# Co2-Messgeraet
Es wurde ein Co2 Messgerät entwickelt. Es wird ein SEN-CSS811V1 Sensor von einem ESP32 eingelesen. Dazu läuft auf dem ESP32 ein Webserver. Ein RTC DS1302 liefert die aktuelle Uhrzeit. Jede Minute werden die Werte des Sensors auf einer SD Karte mit Datum und Uhrzeit des RTCs gespeichert. Auf einem Display wird immer der aktuelle CO2 Gehalt eingelesen
