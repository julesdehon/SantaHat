//
//  CockpitViewController.swift
//  Santa Hat
//
//  Created by Jules Dehon on 22/12/2020.
//  Copyright © 2020 Vanguard Logic LLC. All rights reserved.
//

import UIKit
import CoreBluetooth

// Controls the santa hat controller view
class CockpitViewController: UIViewController, CBPeripheralManagerDelegate {
    
    @IBOutlet weak var titleLabel: UILabel!
    
    @IBOutlet weak var partyModeSection: UIView!
    @IBOutlet weak var colourPickerSection: UIView!
    @IBOutlet weak var trailSection: UIView!
    @IBOutlet weak var solidColourSection: UIView!
    @IBOutlet weak var colourChaseSection: UIView!
    
    @IBOutlet weak var colourWell: UIColorWell!
    
    @IBOutlet weak var trailSizeLabel: UILabel!
    @IBOutlet weak var trailSizeStepper: UIStepper!
    
    @IBOutlet weak var rainbowChaseSwitch: UISwitch!
    
    // Santa hat peripheral
    var peripheralManager: CBPeripheralManager?
    var peripheral: CBPeripheral!
    
    override func viewDidLoad() {
        super.viewDidLoad()
        
        //Create and start the peripheral manager
        peripheralManager = CBPeripheralManager(delegate: self, queue: nil)
        
        // Set title text
        if peripheral.name == nil {
            titleLabel.text = "Connected to nil"
        } else {
            titleLabel.text = "Connected to \(peripheral.name!)"
        }
        
        // Set default colour
        colourWell.selectedColor = .red
        colourWell.supportsAlpha = false
        
        /* Round corners */
        roundCorners()
        
        // Initialise stepper value and label for chase design
        trailSizeStepper.value = 10
        trailSizeLabel.text = Int(trailSizeStepper.value).description
        
        //Disconnect button in nav bar
        self.navigationItem.backBarButtonItem = UIBarButtonItem(title:"Back", style:.plain, target:nil, action:nil)
    }
    
    override func viewDidDisappear(_ animated: Bool) {
        super.viewDidDisappear(animated)
        NotificationCenter.default.removeObserver(self)
    }
    
    func peripheralManagerDidUpdateState(_ peripheral: CBPeripheralManager) {
        if peripheral.state == .poweredOn {
            return
        }
        print("Peripheral manager is running")
    }
    
    func peripheralManagerDidStartAdvertising(_ peripheral: CBPeripheralManager, error: Error?) {
        if let error = error {
            print("\(error)")
            return
        }
    }
    
    /* Actions */
    
    // Trail Effect
    @IBAction func trailSizeChanged(_ sender: UIStepper) {
        trailSizeLabel.text = Int(sender.value).description
    }
    @IBAction func activateTrail(_ sender: UIButton) {
        setColour()
        writeValue(data: "t")
        let size = UInt8(trailSizeStepper.value)
        writeCharacteristic(val: size)
    }
    
    // Party Mode
    @IBAction func partyModePressed(_ sender: UIButton) {
        writeValue(data: "p")
    }
    
    // Clear Santa hat
    @IBAction func clearPressed(_ sender: UIButton) {
        writeValue(data: "c")
    }
    
    // Set santa hat to solid colour
    @IBAction func activateSolidColour(_ sender: UIButton) {
        setColour()
        writeValue(data: "f")
    }
    
    // Chase effect
    @IBAction func activateChase(_ sender: UIButton) {
        setColour()
        writeValue(data: "r")
        let rainbow: UInt8 = rainbowChaseSwitch.isOn ? 1 : 0
        writeCharacteristic(val: rainbow)
    }
    
    
    /* Sending data to santa hat */
    
    func writeValue(data: String){
        let valueString = (data as NSString).data(using: String.Encoding.utf8.rawValue)
        //change the "data" to valueString
        if let blePeripheral = blePeripheral{
            if let txCharacteristic = txCharacteristic {
                blePeripheral.writeValue(valueString!, for: txCharacteristic, type: CBCharacteristicWriteType.withResponse)
            }
        }
    }
    
    func writeCharacteristic(val: UInt8){
        var val = val
        let ns = NSData(bytes: &val, length: MemoryLayout<UInt8>.size)
        blePeripheral!.writeValue(ns as Data, for: txCharacteristic!, type: CBCharacteristicWriteType.withResponse)
    }
    
    
    /* Utility methods */
    
    // Rounds corners of all the controller sections
    func roundCorners() {
        colourPickerSection.layer.cornerRadius = 25
        colourPickerSection.layer.masksToBounds = true
        
        partyModeSection.layer.cornerRadius = 25
        partyModeSection.layer.masksToBounds = true
        
        trailSection.layer.cornerRadius = 25
        trailSection.layer.masksToBounds = true
        
        solidColourSection.layer.cornerRadius = 25
        solidColourSection.layer.masksToBounds = true
        
        colourChaseSection.layer.cornerRadius = 25
        colourChaseSection.layer.masksToBounds = true
    }
    
    // Sets santa hat 'remembered' colour
    func setColour() {
        var red: CGFloat = 0
        var green: CGFloat = 0
        var blue: CGFloat = 0
        var alpha: CGFloat = 0
        
        colourWell.selectedColor!.getRed(&red, green: &green, blue: &blue, alpha: &alpha)
        let r = UInt8(round(red * 255))
        let g = UInt8(round(green * 255))
        let b = UInt8(round(blue * 255))
        
        writeValue(data: "s")
        writeCharacteristic(val: r)
        writeCharacteristic(val: g)
        writeCharacteristic(val: b)
    }
}
