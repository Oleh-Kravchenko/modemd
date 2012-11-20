aplay -c 1 -t raw -r 8000 -f S16_LE /dev/ttyUSB1

AT^SYSCFG=$mode,$acqOrder,$band,$roam,$srvDomain

$mode
2=Auto-Select
13=GSM only
14=WCDMA only
16=no Change

$acqOrder
0=Automatic
1=GSM prefered
2=WCDMA prefered
3=no Change

$band
3fffffff = All
other (query list with "AT^SYSCFG=?")

$roam
0=Not Supported
1=Supported
2=no Change

$srvDomain
0=Circuit-Switched only
1=Packet-Switched only
2=Circuit- & Packet-Switched
3=Any
4=no Change



# AT commands traces

   ^RSSI:15  
 AT+CGMM 
   ^RSSI:15  
   E1550    OK  
 AT+CGMM 
   E1550    OK  
 AT+CMEE=1 
   OK  
 ATE0 
   OK  
 AT^HS=0,0 
   ^HS:40027477,0,0,0,87    OK  
 AT+CFUN? 
   +CFUN: 1    OK  
 AT+CLCC 
   OK  
 AT+CLCK="SC",2 
   +CLCK: 1    OK  
 AT+CPIN? 
   +CPIN: READY    OK  
 AT^SYSINFO 
   ^SYSINFO:2,1,0,5,1,,4    OK  
 AT+CLCK="SC",2 
   +CLCK: 1    OK  
 AT+CGMM 
   E1550    OK  
 AT^CARDLOCK? 
   ^CARDLOCK: 2,10,0    OK  
 AT+CPBS? 
   +CPBS: "SM",0,500    OK  
 AT+CLCK="SC",2 
   +CLCK: 1    OK  
 AT+CLCK="SC",2 
   +CLCK: 1    OK  
 AT+CLCK="SC",2 
   +CLCK: 1    OK  
 AT+CLCK="SC",2 
   +CLCK: 1    OK  
 AT+CLCK="SC",2 
   +CLCK: 1    OK  
 AT+CPMS? 
   +CPMS: "SM",0,40,"SM",0,40,"SM",0,40    OK  
 AT+CPMS="SM","SM","SM" 
   +CPMS: 0,40,0,40,0,40    OK  
 AT+CMGD=? 
   +CMGD: (),(0-4)    OK  
 AT+CLCK="SC",2 
   +CLCK: 1    OK  
 AT^SYSINFO 
   ^SYSINFO:2,1,0,5,1,,4    OK  
 AT^RFSWITCH? 
   ERROR  
 AT+CLCK="SC",2 
   +CLCK: 1    OK  
 AT^CPIN? 
   ^CPIN: READY,,10,3,10,0    OK  
 AT^RFSWITCH? 
   ERROR  
 AT+CLCK="SC",2 
   +CLCK: 1    OK  
 AT+CIMI 
   255071013516311    OK  
 AT+CLIP=1 
   OK  
 AT+CREG=1 
   OK  
 AT+CGREG=1 
   OK  
 AT^RFSWITCH? 
   ERROR  
 AT+CSSN=1,1 
   OK  
 AT^SYSINFO 
   ^SYSINFO:2,1,0,5,1,,4    OK  
 AT+CCWA=1 
   OK  
 AT+CLCK="SC",2 
   +CLCK: 1    OK  
 AT^SYSINFO 
   ^SYSINFO:2,1,0,5,1,,4    OK  
 AT+CIMI 
   255071013516311    OK  
 AT+CLCK="SC",2 
   +CLCK: 1    OK  
 AT^SYSINFO 
   ^SYSINFO:2,1,0,5,1,,4    OK  
 AT+CNMI=2,1,2,2,0 
   OK  
 AT^CVOICE=? 
   ^CVOICE:(0)    OK  
 AT+CMGF=0 
   OK  
 AT^CVOICE? 
   ^CVOICE:0,8000,16,20    OK  
 AT+CLCK="SC",2 
   +CLCK: 1    OK  
 AT+CLCK="SC",2 
   +CLCK: 1    OK  
 AT+CPMS="SM","SM","SM" 
   +CPMS: 0,40,0,40,0,40    OK  
 AT^CSNR? 
   ^CSNR:-83,-7    OK  
 AT+CPMS? 
   +CPMS: "SM",0,40,"SM",0,40,"SM",0,40    OK  
 AT^SYSINFO 
   ^SYSINFO:2,1,0,5,1,,4    OK  
 AT+CLCK="SC",2 
   +CLCK: 1    OK  
 AT^SYSINFO 
   ^SYSINFO:2,1,0,5,1,,4    OK  
 AT+CPBS="SM" 
   OK  
 AT+COPS=3,0 
   OK  
 AT+CPBS? 
   +CPBS: "SM",0,500    OK  
 AT+COPS? 
   +COPS: 0,0,"Ukrtelecom",2    OK  
 AT+CPBS="SM" 
   OK  
 AT+COPS=3,2 
   OK  
 AT^CPBR=? 
   ^CPBR: (1-500),24,60    OK  
 AT+COPS? 
   +COPS: 0,2,"25507",2    OK  
 AT^CVOICE=? 
   ^CVOICE:(0)    OK  
 AT^CVOICE? 
   ^CVOICE:0,8000,16,20    OK  
 AT+CLVL? 
   +CLVL: 4    OK  
 AT+CLVL=? 
   +CLVL: (0-5)    OK  
 AT+CLVL? 
   +CLVL: 4    OK  
 AT+CLVL=? 
   +CLVL: (0-5)    OK  
 AT+CLVL=4 
   OK  
 AT+CLVL=? 
   +CLVL: (0-5)    OK  
 AT+CLCK="SC",2 
   +CLCK: 1    OK  
 AT+CLCK="SC",2 
   +CLCK: 1    OK  
 AT+CLCK="SC",2 
   +CLCK: 1    OK  
 AT+CPMS? 
   +CPMS: "SM",0,40,"SM",0,40,"SM",0,40    OK  
 AT+CPMS="SM","SM","SM" 
   +CPMS: 0,40,0,40,0,40    OK  
 AT+CMGD=? 
   +CMGD: (),(0-4)    OK  
 AT+CLCK="SC",2 
   +CLCK: 1    OK  
 AT+CPBS="SM" 
   OK  
 AT^CPBR=? 
   ^CPBR: (1-500),24,60    OK  
 AT^CPBR=1,50 
   +CME ERROR: 22  
 AT^CPBR=51,100 
   +CME ERROR: 22  
 AT^CPBR=101,150 
   +CME ERROR: 22  
 AT^CPBR=151,200 
   +CME ERROR: 22  
 AT^CPBR=201,250 
   +CME ERROR: 22  
 AT^CPBR=251,300 
   +CME ERROR: 22  
 AT^CPBR=301,350 
   +CME ERROR: 22  
 AT^CPBR=351,400 
   +CME ERROR: 22  
 AT^CPBR=401,450 
   +CME ERROR: 22  
 AT^CPBR=451,500 
   +CME ERROR: 22  
 AT^CVOICE=? 
   ^CVOICE:(0)    OK  
 AT^CVOICE? 
   ^CVOICE:0,8000,16,20    OK  
 AT+CLVL? 
   +CLVL: 4    OK  
 AT+CLVL=? 
   +CLVL: (0-5)    OK  
 AT+CLVL? 
   +CLVL: 4    OK  
 AT+CLVL=? 
   +CLVL: (0-5)    OK  
 AT+CLVL=4 
   OK  
 AT+CLVL=? 
   +CLVL: (0-5)    OK  
 AT+CIMI 
   255071013516311    OK  
 AT^RFSWITCH? 
   ERROR  
   ^RSSI:15  
   ^RSSI:15  
   ^RSSI:15  
 AT^SYSINFO 
   ^SYSINFO:2,1,0,5,1,,4    OK  
 AT^CSNR? 
   ^CSNR:-84,-9    OK  
 AT+COPS=3,0 
   OK  
 AT+COPS? 
   +COPS: 0,0,"Ukrtelecom",2    OK  
 AT+COPS=3,2 
   OK  
 AT+COPS? 
   +COPS: 0,2,"25507",2    OK  
 AT^RFSWITCH? 
   ERROR  
   ^RSSI:15  
   ^RSSI:15  
   ^RSSI:15  
 AT^SYSINFO 
   ^SYSINFO:2,1,0,5,1,,4    OK  
 AT^CSNR? 
   ^CSNR:-83,-11    OK  
 AT+COPS=3,0 
   OK  
 AT+COPS? 
   +COPS: 0,0,"Ukrtelecom",2    OK  
 AT+COPS=3,2 
   OK  
 AT+COPS? 
   +COPS: 0,2,"25507",2    OK  
 AT^RFSWITCH? 
   ERROR  
 AT^DDSETEX=2 
   OK  
 ATD+380972763224; 
   OK  
   ^ORIG:1,0  
   ^RSSI:15  
   ^CONF:1  
 AT^DDSETEX=2 
   OK  
 AT^SYSINFO 
   ^SYSINFO:2,1,0,5,1,,4    OK  
 AT^CSNR? 
   ^CSNR:-82,-11    OK  
 AT+COPS=3,0 
   OK  
 AT+COPS? 
   +COPS: 0,0,"Ukrtelecom",2    OK  
 AT+COPS=3,2 
   OK  
 AT+COPS? 
   +COPS: 0,2,"25507",2    OK  
 AT^RFSWITCH? 
   ERROR  
   ^RSSI:15  
   ^RSSI:15  
   ^RSSI:15  
   ^CONN:1,0  
   ^CEND:1,0,104,31  
   ^RSSI:15  
 AT+CLCK="SC",2 
   +CLCK: 1    OK  
 AT+CPBS? 
   +CPBS: "SM",0,500    OK  
 AT+CPBS? 
   +CPBS: "SM",0,500    OK  
 AT^CPBR=? 
   ^CPBR: (1-500),24,60    OK  
 AT^SYSINFO 
   ^SYSINFO:2,1,0,5,1,,4    OK  
 AT^SYSINFO 
   ^SYSINFO:2,1,0,5,1,,4    OK  
 AT^SYSINFO 
   ^SYSINFO:2,1,0,5,1,,4    OK  
 AT^CSNR? 
   ^CSNR:-82,-8    OK  
 AT+COPS=3,0 
   OK  
 AT+COPS? 
   +COPS: 0,0,"Ukrtelecom",2    OK  
 AT+COPS=3,2 
   OK  
 AT+COPS? 
   +COPS: 0,2,"25507",2    OK  
 AT^RFSWITCH? 
   ERROR  
   ^RSSI:15  
   ^RSSI:15  
 AT^SYSINFO 
   ^SYSINFO:2,1,0,5,1,,4    OK  
 AT^CSNR? 
   ^CSNR:-83,-11    OK  
 AT+COPS=3,0 
   OK  
 AT+COPS? 
   +COPS: 0,0,"Ukrtelecom",2    OK  
 AT+COPS=3,2 
   OK  
 AT+COPS? 
   +COPS: 0,2,"25507",2    OK  
 AT^RFSWITCH? 
   ERROR  
   ^RSSI:15  
