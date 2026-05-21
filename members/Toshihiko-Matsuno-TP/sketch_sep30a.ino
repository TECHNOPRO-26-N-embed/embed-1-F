const int button_pin_coin=2;
const int button_pin_buy=3;
const int led_pin_sell=12;
const int led_pin_product=13;
int coin=0;
int buy=0;

void setup(){
  pinMode(button_pin_coin,INPUT_PULLUP);
  pinMode(button_pin_buy,INPUT_PULLUP);
  pinMode(led_pin_sell,OUTPUT);
  pinMode(led_pin_product,OUTPUT);
  attachInterrupt(0,pushed,FALLING);
  attachInterrupt(1,pushed2,FALLING);
  Serial.begin(9600);
}

void loop(){
  if(coin>=3){
    digitalWrite(led_pin_sell,HIGH);
    if(buy!=0){
      digitalWrite(led_pin_product,HIGH);
      delay(2000);
      digitalWrite(led_pin_product,LOW);
      digitalWrite(led_pin_sell,LOW);
      coin=coin-3;
      buy=0;
      Serial.println(coin);
      coin=0;
    }
  }else{
    buy=0;
  }
}

void pushed(){
  coin=coin+1;
  Serial.println(coin);
}

void pushed2(){
  buy=buy+1;
}
  
