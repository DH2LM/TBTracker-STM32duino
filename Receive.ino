/************************************************************************************
* Process a received LoRa packet. 
* This function will run if a packect has been received
* You should create your own actions here.
* Be sure to check if a correct packet was received!
************************************************************************************/
void ProcessRXPacket()
{
  // Buffer to hold the received data from the radio
  byte buf[PACKETLEN];
  int16_t state;

  // EXAMPLE OF RX ACTION 

  // reset the data received flag
  // SerialDebug.println("============================================");
  // SerialDebug.println("PACKET RECEIVED!");
  // SerialDebug.println("============================================");
  
  // Grab the data from the radio module
  switch(LORA_MODE)
  {
     case 1:  // Implicit header, so tell the radio how many bytes to read
        state = radio.readData(buf,LoRaSettings.implicitHeader);
     break;
     default: 
        state = radio.readData(buf,0);
    break;
  }

  // Packet was successfully received
  if (state == RADIOLIB_ERR_NONE) 
  {
      // SerialDebug.print( "Packet length: "); // SerialDebug.println(radio.getPacketLength());
      // SerialDebug.print("          RSSI: "); // SerialDebug.println(radio.getRSSI());
      // SerialDebug.print("           SNR: "); // SerialDebug.println(radio.getSNR());
      // SerialDebug.print("First 10 chars: ");
      // Print the first 10 hex chars of the packet
      // SerialDebug.print("[RADIO] first 10 hex chars:\t");

      digitalWrite(LED_GRN, LOW);
      digitalWrite(LED_RED, HIGH);

      for (int i = 0; i < 10; i++)
      {
        // SerialDebug.print(buf[i],HEX);
        // SerialDebug.print(" ");
        delay(10);
        digitalWrite(LED_GRN, !digitalRead(LED_GRN));
        digitalWrite(LED_RED, !digitalRead(LED_RED));
      }

      digitalWrite(LED_RED, LOW);
      digitalWrite(LED_GRN, HIGH);

      // SerialDebug.println();
      // SerialDebug.println("============================================");
  }

  // Reset the received flag  
  receivedFlag = false;
}
