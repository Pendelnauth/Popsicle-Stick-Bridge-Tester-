
bool Write_Flash(String To_Write){//write should occur all at once if at all possible
  noInterrupts();
  File f = LittleFS.open("data.csv", "a");//append?
  if(f){
    f.print(To_Write);
    f.close();
    interrupts();
    return true;
  }else{
    interrupts();
    return false;//return if read failed
  }
}

// Called when the USB stick connected to a PC and the drive opened
// Note this is from a USB IRQ so no printing to SerialUSB/etc.
void plug(uint32_t i) {
  (void) i;
  okayToWrite = false;
}

// Called when the USB is ejected or removed from a PC
// Note this is from a USB IRQ so no printing to SerialUSB/etc.
void unplug(uint32_t i) {
  (void) i;
  okayToWrite = true;
}

// Called when the PC tries to delete the single file
// Note this is from a USB IRQ so no printing to SerialUSB/etc.
void deleteCSV(uint32_t i) {
  (void) i;
  LittleFS.remove("data.csv");
  File f = LittleFS.open("data.csv", "w");//write
  f.print("Load Cell 0, Load Cell 1, Total load\n");
  f.close();
}