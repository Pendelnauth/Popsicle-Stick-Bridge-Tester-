


void Start_Motor(){
  digitalWrite(NSleep, 1); //Enable motor driver
}

void Update_Motor(uint8_t speed, bool dir){
  speed = map(speed, 0,255,130,255);
  if(dir){
      analogWrite(IN1, speed);
      digitalWrite(IN2, LOW);
  }else{
      digitalWrite(IN1, LOW);
      analogWrite(IN2, speed);
  }
}

void Stop_Motor(){
  digitalWrite(IN1, LOW);
  digitalWrite(IN2, LOW);
  digitalWrite(NSleep, 0); //disable motor driver
}

float Motor_Current(){
  return (float(analogRead(IDrive) - 36)/496.4);
}
















