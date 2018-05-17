void pinyPIR(){
  pinMode(PIR1, INPUT);
  pinMode(PIR2, INPUT);
}


boolean czujniki_odczyt(int PIR_pin)
{

if(digitalRead(PIR_pin) == HIGH){
  return true;
}else{
return false;
}
}

