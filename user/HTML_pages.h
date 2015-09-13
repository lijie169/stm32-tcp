/******************************************************************************************
   HTML_pages.h (v1.0)
-------------------------------------------------------------------------------------
This code is from the book:
"Embedded Internet: TCP/IP Basics, Implementation and Applications" by Sergio Scaglia
[Pearson Education, 2006 - ISBN: 0-32-130638-4]

This code is copyright (c) 2006 by Sergio Scaglia, and it may only be used for educational
purposes.  For commercial use, please contact me at sscaglia@intramarket.com.ar
For more information and updates, please visit www.embeddedinternet.org
******************************************************************************************/

#ifdef LIMIT_32K

const char about[]   = "<...>";
const char applic[]  = "<...>";
const char index[]   = "<...>";
const char mail[]    = "<...>";
const char sent[]    = "<...>";
const char setup[]   = "<...>";
const char setupok[] = "<...>";

#else

const char index[] =
"<HTML><HEAD><TITLE>Embedded Internet</TITLE></HEAD>"
"<BODY><a href=http://www.embeddedinternet.org><img src=logo.gif border=0></a><hr>"
"<font size=5><b>Web Control Panel</b></font>"
"<table cellspacing=0><tr>"
"<td align=center bgcolor=blue width=300><a href=index.htm  ><b><font face=Arial size=2 color=#FFFFFF>HOME</font></b></a></td>"
"<td align=center bgcolor=blue width=300><a href=setup.htm  ><b><font face=Arial size=2 color=#FFFFFF>Setup</font></b></a></td>"
"<td align=center bgcolor=blue width=300><a href=applic.htm  ><b><font face=Arial size=2 color=#FFFFFF>Application</font></b></a></td>"
"<td align=center bgcolor=blue width=300><a href=mail.htm  ><b><font face=Arial size=2 color=#FFFFFF>Send e-mail</font></b></a></td>"
"<td align=center bgcolor=blue width=300><a href=about.htm  ><b><font face=Arial size=2 color=#FFFFFF>About</font></b></a></td>"
"</tr></table>"
"</BODY></HTML>";

const char setup[] =
"<HTML><HEAD><TITLE>Embedded Internet</TITLE></HEAD>"
"<BODY><a href=http://www.embeddedinternet.org><img src=logo.gif border=0></a><hr>"
"<font size=5><b>Web Control Panel</b></font>"
"<table cellspacing=0><tr>"
"<td align=center bgcolor=blue width=300><a href=index.htm  ><b><font face=Arial size=2 color=#FFFFFF>Home</font></b></a></td>"
"<td align=center bgcolor=blue width=300><a href=setup.htm  ><b><font face=Arial size=2 color=#FFFFFF>SETUP</font></b></a></td>"
"<td align=center bgcolor=blue width=300><a href=applic.htm  ><b><font face=Arial size=2 color=#FFFFFF>Application</font></b></a></td>"
"<td align=center bgcolor=blue width=300><a href=mail.htm  ><b><font face=Arial size=2 color=#FFFFFF>Send e-mail</font></b></a></td>"
"<td align=center bgcolor=blue width=300><a href=about.htm  ><b><font face=Arial size=2 color=#FFFFFF>About</font></b></a></td>"
"</tr></table>"
"<h3>SETUP PAGE</h3><form action=setupok.htm method=post>"
"<hr><h3>Real Time Clock</h3>"
"DATE: <select name=dow>"
"<option selected>&20</option>"
"<option>Sunday</option>"
"<option>Monday</option>"
"<option>Tuesday</option>"
"<option>Wednesday</option>"
"<option>Thursday</option>"
"<option>Friday</option>"
"<option>Saturday</option>"
"</select> , <select name=month>"
"<option selected>&21</option>"
"<option>January</option>"
"<option>February</option>"
"<option>March</option>"
"<option>April</option>"
"<option>May</option>"
"<option>June</option>"
"<option>July</option>"
"<option>August</option>"
"<option>September</option>"
"<option>October</option>"
"<option>November</option>"
"<option>December</option>"
"</select> <input type=text name=dom value=&22 size=1 maxlength=2> ,  <input type=text name=year value=&23 size=3 maxlength=4><br>"
"TIME:  <input type=text name=hour value=&10 size=1 maxlength=2>:<input type=text name=min value=&11 size=1 maxlength=2>:<input type=text name=sec value=&12 size=1 maxlength=2><br>"
"<hr><h3>Led Control</h3><input type=checkbox name=led value=on &24>Led On"
"<hr><input type=submit name=send value=Update><input type=submit name=send value=Cancel><br>"
"</form><a href=index.htm >Home</a></BODY></HTML>";


const char setupok[] =
"<HTML><HEAD><TITLE>Embedded Internet</TITLE></HEAD>"
"<BODY><a href=http://www.embeddedinternet.org><img src=logo.gif border=0></a><hr>"
"<font size=5><b>Web Control Panel</b></font>"
"<table cellspacing=0><tr>"
"<td align=center bgcolor=blue width=300><a href=index.htm  ><b><font face=Arial size=2 color=#FFFFFF>Home</font></b></a></td>"
"<td align=center bgcolor=blue width=300><a href=setup.htm  ><b><font face=Arial size=2 color=#FFFFFF>SETUP</font></b></a></td>"
"<td align=center bgcolor=blue width=300><a href=applic.htm  ><b><font face=Arial size=2 color=#FFFFFF>Application</font></b></a></td>"
"<td align=center bgcolor=blue width=300><a href=mail.htm  ><b><font face=Arial size=2 color=#FFFFFF>Send e-mail</font></b></a></td>"
"<td align=center bgcolor=blue width=300><a href=about.htm  ><b><font face=Arial size=2 color=#FFFFFF>About</font></b></a></td>"
"</tr></table>"
"<h4>The data was updated!</h4>"
"<hr><h3>Real Time Clock</h3><br>DATE: &20, &21 &22 &23<br>TIME:  &10:&11:&12"
"<hr><h3>Led Control</h3><br>LED &25<hr></BODY></HTML>";


const char applic[] =
"<HTML><HEAD><TITLE>Embedded Internet</TITLE>&66&67</HEAD>"
"<BODY><a href=http://www.embeddedinternet.org><img src=logo.gif border=0></a><hr>"
"<font size=5><b>Web Control Panel</b></font>"
"<table cellspacing=0><tr>"
"<td align=center bgcolor=blue width=300><a href=index.htm  ><b><font face=Arial size=2 color=#FFFFFF>Home</font></b></a></td>"
"<td align=center bgcolor=blue width=300><a href=setup.htm  ><b><font face=Arial size=2 color=#FFFFFF>Setup</font></b></a></td>"
"<td align=center bgcolor=blue width=300><a href=applic.htm  ><b><font face=Arial size=2 color=#FFFFFF>APPLICATION</font></b></a></td>"
"<td align=center bgcolor=blue width=300><a href=mail.htm  ><b><font face=Arial size=2 color=#FFFFFF>Send e-mail</font></b></a></td>"
"<td align=center bgcolor=blue width=300><a href=about.htm  ><b><font face=Arial size=2 color=#FFFFFF>About</font></b></a></td>"
"</tr></table>"
"<h3 align=center>APPLICATION WEB CONTROL PANEL</h3><table border=0 cellpadding=0 cellspacing=0><tr align=center valign=bottom>"
"<td><img src=black.gif height=2 width=60><br><img src=level.gif height=6 width=60 ><br><img src=black.gif height=3 width=60 ><br><img src=clear.gif height=22 width=60></td><td>"
"<img src=black.gif height=60 width=3><br><img src=level.gif height=6 width=3 ><br><img src=black.gif height=3 width=3 ><br><img src=clear.gif height=22 width=3></td>"
"<td><img src=valve.jpg height=100 width=30 alt='Valve 1: &59'><br>&50<img src=black.gif height=3 width=30><br><img src=clear.gif height=22 width=30></td>"
"<td><img src=black.gif height=60 width=3><br><img src=level.gif height=6 width=3 ><br><img src=black.gif height=3 width=3 ><br><img src=clear.gif height=22 width=3></td>"
"<td><img src=black.gif height=2 width=30><br><img src=level.gif height=6 width=30 ><br><img src=black.gif height=3 width=30 ><br><img src=clear.gif height=22 width=30></td>"
"<td><img src=&51 alt='Pump 1: &59'><br><img src=clear.gif height=15 width=30></td>"
"<td><img src=black.gif height=2 width=30><br><img src=level.gif height=6 width=30 ><br><img src=black.gif height=3 width=30 ><br><img src=clear.gif height=22 width=30></td>"
"<td><img src=black.gif height=200 width=4><br><img src=level.gif height=6 width=4 ><br><img src=black.gif height=3 width=4 ><br><img src=clear.gif height=22 width=4></td>"
"<td><img src=level.gif height=&52 width=120 alt='Liquid Level: &62 lts'><br><img src=level.gif height=6 width=120 ><br><img src=black.gif height=3 width=120><br><img src=clear.gif height=22 width=120></td>"
"<td><img src=black.gif height=200 width=4><br><img src=level.gif height=6 width=4 ><br><img src=black.gif height=3 width=4 ><br><img src=clear.gif height=22 width=4></td>"
"<td><img src=black.gif height=2 width=30><br><img src=level.gif height=6 width=30 ><br><img src=black.gif height=3 width=30 ><br><img src=clear.gif height=22 width=30></td>"
"<td><img src=&53 alt='Pump 2: &61'><br><img src=clear.gif height=15 width=30></td>"
"<td><img src=black.gif height=2 width=30><br><img src=level.gif height=6 width=30 ><br><img src=black.gif height=3 width=30 ><br><img src=clear.gif height=22 width=30></td>"
"<td><img src=black.gif height=60 width=3><br><img src=level.gif height=10 width=3 ><br><img src=black.gif height=3 width=3 ><br><img src=clear.gif height=22 width=3></td>"
"<td><img src=valve.jpg height=100 width=30 alt='Valve 2: &61'><br>&54<img src=black.gif height=3 width=30><br><img src=clear.gif height=22 width=30></td>"
"<td><img src=black.gif height=60 width=3><br><img src=level.gif height=6 width=3 ><br><img src=black.gif height=3 width=3 ><br><img src=clear.gif height=22 width=3></td>"
"<td><img src=black.gif height=2 width=60><br><img src=level.gif height=6 width=60 ><br><img src=black.gif height=3 width=60 ><br><img src=clear.gif height=22 width=60></td>"
"<td><table border=0 cellpadding=0 cellspacing=0><tr align=center valign=bottom><td ><b>TEMP</b><br><img src=scale.gif align=right></td>"
"<td><img src=mercury.gif height=&55 width=5 alt='Temp: &63 �C'></td><td><img src=black.gif height=200 width=1></td><td><img src=clear.gif height=10 width=20></td></table></td>"
"<td valign=top align=left><table border=0 cellpadding=0 cellspacing=0><tr><td align=left valign=top>"
"<table border=3 bgcolor=#CDCDCD cellpadding=2 cellspacing=0>"
"<tr><td align=right valign=middle><font color=#8C7853><b>STATE:</b></font></td><td align=left valign=middle><font color=#8C7853><b>&49</b></font></td></tr>"
"<tr><td align=right><b>Valve 1:<br>Pump 1:<br>Heater:<br>Valve 2:<br>Pump 2:</b></td><td align=left><font color=#2F4F4F><b>&59<br>&59<br>&60<br>&61<br>&61<br></b></font></td></tr>"
"<tr><td align=right><b>Liquid Level:<br>Temperature:</b></td><td align=left><font color=#4D4DFF><b>&62 lts<br>&63 �C</b></font></b></td></tr></table></td></tr>"
"<tr><td><form action=applic.htm method=post><table bgcolor=#FFFFCC border=3 width=100%><tr><td><input type=submit name=options value=Start></td></tr><tr><td><input type=checkbox name=cycle value=Cycle &64>Cycle continuously</td></tr><tr><td><input type=checkbox name=autorefresh value=Autorefresh &65>Autorefresh<br>    every <input type=text name=refreshtime value=&68 maxlength=2 size=2> seconds</td></tr><tr><td><input type=submit name=options value='Refresh'></td></tr></table></form></td></tr>"
"</table></td></tr></td></tr></table><table border=0 cellpadding=0 cellspacing=0><tr><td valign=top><img src=&56 width=207></td><td align=middle valign=topwidth=132><img src=&57 width=80 height=60 alt='Heater: &60'></td><td valign=top><img src=&58 width=200></td></tr></table>"
"</body></html>";


const char mail[] =
"<HTML><HEAD><TITLE>Embedded Internet</TITLE></HEAD>"
"<BODY><a href=http://www.embeddedinternet.org><img src=logo.gif border=0></a><hr>"
"<font size=5><b>Web Control Panel</b></font>"
"<table cellspacing=0><tr>"
"<td align=center bgcolor=blue width=300><a href=index.htm  ><b><font face=Arial size=2 color=#FFFFFF>Home</font></b></a></td>"
"<td align=center bgcolor=blue width=300><a href=setup.htm  ><b><font face=Arial size=2 color=#FFFFFF>Setup</font></b></a></td>"
"<td align=center bgcolor=blue width=300><a href=applic.htm  ><b><font face=Arial size=2 color=#FFFFFF>Application</font></b></a></td>"
"<td align=center bgcolor=blue width=300><a href=mail.htm  ><b><font face=Arial size=2 color=#FFFFFF>SEND e-mail</font></b></a></td>"
"<td align=center bgcolor=blue width=300><a href=about.htm  ><b><font face=Arial size=2 color=#FFFFFF>About</font></b></a></td>"
"</tr></table>"
"<h3 align=center>SEND e-mail FORM</h3><br>"
"<FORM action=sent.htm method=post>"
"<TABLE align=center>"
"<tr><td align=right><b>To:</b></td><td><input type=text name=to maxlength=50 size=30></td></tr>"
"<tr><td align=right><b>Subject:</b></td><td><input type=text name=subject maxlength=50 size=30></td></tr>"
"<tr><td align=right valign=top><b>Message:<br>(Max.Length<br>=200 chars)</b></td><td><TEXTAREA name=message cols=30 rows=6></TEXTAREA></td></tr>"
"<tr><td align=right colspan=2><input type=submit name=mail value=Send></td></tr>"
"</TABLE></FORM></BODY></HTML>";

const char sent[] =
"<HTML><HEAD><TITLE>Embedded Internet</TITLE></HEAD>"
"<BODY><a href=http://www.embeddedinternet.org><img src=logo.gif border=0></a><hr>"
"<font size=5><b>Web Control Panel</b></font>"
"<table cellspacing=0><tr>"
"<td align=center bgcolor=blue width=300><a href=index.htm  ><b><font face=Arial size=2 color=#FFFFFF>Home</font></b></a></td>"
"<td align=center bgcolor=blue width=300><a href=setup.htm  ><b><font face=Arial size=2 color=#FFFFFF>Setup</font></b></a></td>"
"<td align=center bgcolor=blue width=300><a href=applic.htm  ><b><font face=Arial size=2 color=#FFFFFF>Application</font></b></a></td>"
"<td align=center bgcolor=blue width=300><a href=mail.htm  ><b><font face=Arial size=2 color=#FFFFFF>SEND e-mail</font></b></a></td>"
"<td align=center bgcolor=blue width=300><a href=about.htm  ><b><font face=Arial size=2 color=#FFFFFF>About</font></b></a></td>"
"</tr></table>"

"<h4>The e-mail was sent!</h4></BODY></HTML>";


const char about[] =
"<HTML><HEAD><TITLE>Embedded Internet</TITLE></HEAD>"
"<BODY><a href=http://www.embeddedinternet.org><img src=logo.gif border=0></a><hr>"
"<font size=5><b>Web Control Panel</b></font>"
"<table cellspacing=0><tr>"
"<td align=center bgcolor=blue width=300><a href=index.htm  ><b><font face=Arial size=2 color=#FFFFFF>Home</font></b></a></td>"
"<td align=center bgcolor=blue width=300><a href=setup.htm  ><b><font face=Arial size=2 color=#FFFFFF>Setup</font></b></a></td>"
"<td align=center bgcolor=blue width=300><a href=applic.htm  ><b><font face=Arial size=2 color=#FFFFFF>Application</font></b></a></td>"
"<td align=center bgcolor=blue width=300><a href=mail.htm  ><b><font face=Arial size=2 color=#FFFFFF>Send e-mail</font></b></a></td>"
"<td align=center bgcolor=blue width=300><a href=about.htm  ><b><font face=Arial size=2 color=#FFFFFF>ABOUT</font></b></a></td>"
"</tr></table>"
"<h3 align=center>THE EMBEDDED INTERNET BOOK INDEX</h3><br>"
"<TABLE><tr><td valign=top align=center><a href=http://www.intramarket.com.ar/resume><img src=sergio.jpg border=0></a><br><br>"
"<a href=http://www.aw-bc.com><img src=pearson.gif border=0></a><br><br><a href=http://www.iar.com><img src=iar.gif border=0></a><br><br>"
"<a href=http://www.nxp.com><img src=philips.jpg border=0></a><br><br><a href=http://www.phaedsys.org><img src=psl_logo.gif border=0></a></td><td width=100></td>"
"<td>"
"<h3>Embedded Internet<br>"
"TCP/IP Basics, Implementation and Applications<br></h3>"

"<h4 align=right>by Sergio Scaglia<br></h4>"
"<b>TABLE OF CONTENTS<br><hr></b>"

"<b>Preface</b><br>"

"<b>PART  I:	TCP/IP basics</b><br>"
"1.	Introduction to Networking<br>"
"2.	The TCP/IP Stack<br>"
"3.	LAN Technologies: Ethernet<br>"
"4.	Network Interface: SLIP and PPP<br>"
"5.	The Internet layer: IP and ICMP<br>"
"6.	The Transport Layer: UDP and TCP<br>"
"7.	Remote Access: TELNET<br>"
"8.	The File Transfer Protocol: FTP<br>"
"9.	The e-mail Protocols: SMTP and POP3<br>"
"10.	The World Wide Web Protocol: HTTP<br>"
"11.	The Domain Name System<br><br>"

"<b>PART  II:	  Embedded Internet Implementation</b><br>"

"12.	Preparing the Labs<br>"
"13.	The Application and the TCP/IP Stack<br>"
"14.	Connecting to a LAN: Ethernet and ARP<br>"
"15.	Dial-Up Networking: PPP<br>"
"16.	Implementing the IP layer (IP and ICMP)<br>"
"17.	Implementing the Transport layer (UDP and TCP)<br>"
"18.	UDP-based and TCP-based Embedded Server Applications<br>"
"19.	Sending e-mail messages: SMTP<br>"
"20.	Serving the World Wide Web: HTTP<br><br>"

"<b>PART  III:   Embedded Internet Applications</b><br>"

"21.	Remote Monitoring, Access, and Control<br>"
"22.	Security and Surveillance Applications<br>"
"23.	Tracking Applications<br>"
"24.	Outsourcing embedded code: using Web Services<br><br>"

"<b>APPENDICES</b><br>"

"Appendix A:   Bluetooth and IEEE 802.11 Wireless LAN<br>"
"Appendix B:   Internet Next Generation:  IPv6<br>"
"Appendix C:   Dynamic Host Configuration Protocol<br>"
"Appendix D:   Simple Network Management Protocol<br>"
"Appendix E:   Administrator Utilities<br>"
"Appendix F:   Network Protocol Analyzer: Ethereal"
"</tr></TABLE></BODY></HTML>";

#endif

