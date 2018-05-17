void gsmCommand (String cmd)
{
  gsm.print(cmd);
  gsm.print((char)13);
}

void gsmSMS (String phone, String msg)
{
  gsm.flush();
  delay(100);
  gsmCommand("AT+CMGS=\"" + phone + "\"");
  delay(100);
  gsm.print(msg);
  delay(100);
  gsm.print((char)26);
}


void odczytanie()
{
memset(odczyt, 0, sizeof(odczyt));
//  if (gsm.available()) {
//    memset(odczyt, 0, sizeof(odczyt));
//      for(int i=0;i<100;i++){
//        if(gsm.available()){
//      odczyt[i] = gsm.read();
//      delay(5);
//    }
//    
//    }
    int i = 0;
    while (gsm.available()) {
      odczyt[i] = gsm.read();
      i++;
     delay(4);
    }
  
  }





