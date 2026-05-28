# Sistem inteligent de monitorizare a luminii si control al ventilatorului folosind ATmega328P

Acest proiect a fost realizat folosind microcontrollerul ATmega328P si are la baza repository-ul de Embedded Systems utilizat in cadrul Facultatii de Automatica, Calculatoare si Electronica, Universitatea din Craiova.

Sistemul monitorizeaza nivelul de lumina cu ajutorul unui senzor si controleaza automat LED-uri, buzzer-ul si ventilatorul.
De asemenea, a fost realizata o interfata grafica in Python pentru monitorizare in timp real si control manual al ventilatorului.

---

## Functionalitati

* detectare automata a luminii folosind ADC
* LED verde atunci cand lumina este aprinsa
* LED rosu + avertizare sonora atunci cand lumina este stinsa
* pornirea automata a ventilatorului dupa 5 secunde daca lumina ramane stinsa
* LED galben atunci cand ventilatorul este activ
* afisare mesaje pe LCD 16x2 I2C
* comunicare seriala folosind USART
* interfata grafica realizata in Python
* control manual al ventilatorului din interfata

---

## Componente folosite

* ATmega328P
* Arduino Nano
* senzor de lumina
* LCD 16x2 cu modul I2C
* buzzer
* ventilator
* LED-uri
* comunicare USB Serial

---

## Structura proiectului

```text
├── bsp/                 # definitii pentru placa
├── drivers/             # drivere hardware
│   ├── adc/
│   ├── gpio/
│   ├── interrupt/
│   ├── lcd/
│   ├── pwm/
│   ├── timer/
│   └── usart/
├── src/                 # logica principala a aplicatiei
│   ├── main.c
│   ├── proiect.c
│   └── proiect.h
├── utils/
└── Makefile
```

---

## Functionarea sistemului

### Lumina aprinsa

* LED-ul verde se aprinde
* pe LCD apare:

  ```text
  Light ON
  System normal
  ```

### Lumina stinsa

* LED-ul rosu se aprinde
* buzzer-ul incepe sa emita avertizari sonore
* incepe un countdown de 5 secunde

### Activarea ventilatorului

Daca lumina ramane stinsa dupa countdown:

* LED-ul galben se aprinde
* ventilatorul porneste automat
* pe LCD apare:

  ```text
  Fan ON
  Light OFF
  ```

### Control manual al ventilatorului

Ventilatorul poate fi pornit manual si din interfata Python, chiar daca lumina este aprinsa.

---

## Interfata Python

Interfata grafica a fost realizata separat in PyCharm folosind Python.

Interfata permite:

* monitorizarea sistemului in timp real
* afisarea statusului luminii
* afisarea statusului ventilatorului
* afisarea countdown-ului ventilatorului
* control manual al ventilatorului prin comunicare seriala

---

## Compilare si upload

### Compilare

```bash
make all 
```

### Upload pe placa

```bash
make flash
```

### Stergere fisiere generate

```bash
make clean
```

---

## Membrii echipei

* Andrei Oltita Costinela
* Diaconu Greti Stefania
* Username-urile GitHub: Oltita si greti1