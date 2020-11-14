#  Skill 29 - Security Issues

Author: Nnenna Eze

Date: 2020-11-09
-----

## Summary
This skill did not require any code, it was simply about how to protect the overall system of the remote car that will be used in the future. 

<img src = "https://user-images.githubusercontent.com/44929220/99138608-d1b32980-25ff-11eb-9ffd-d7220c84efb3.png" width="650" height="350">


Identify weaknesses in your overall system (client, local network, internet, server, node.js, ESP32) with respect to security compromis:

Weaknesses in the system are based on the local network, internet, server, node.js and ESP32. In a person’s local network, it may be public
or lack the protection necessary to maintain control/ transfer data during a security attack. Node.js itself poses a security risk and is
open to attacks unless security measures are implemented. Last, the ESP32 connected over wifi can lead to more vulnerabilities. The ESP could 
be sent data that leads it to disconnect or crash from the system.



What ways can a bad guy attack your specific system? – list at least five and be very specific

1. Network intrusion can occur when a user is able to get 
access behind a firewall, especially when protections are lower. If the wifi connection occurs 
over a LAN (Local Area Network), it is likely that the speed is faster and more secure in 
comparison to WAN (Wide Area Network). Once accessed, the attacker can have control over the 
system.

2. Denial of Service can occur when bots launch an overwhelming traffic on a service/ system and 
make it unusable. This could lead to the flooding of unnecessary data to the ports and lead the 
user to lose their remote control over the car on a webpage.

3. Eavesdroppers and data interception can occur if a person gets in close proximity to and taps 
in to the network the car is connected to. This data interception can occur before the data is 
transferred to the sensor or before the data is transferred to the motor, not allowing the car to 
move and let the server know if the car has moved.

4. Spoilers are a deliberate attack on protocols. This can lead to sending bad requests to a server, 
thus not allowing the server to perform the tasks it needs.

5. Endpoint attacks pick on the system and target the entry points into a network, such as targeting 
the sensors, motors, or even the computer. Then, the hacker is able to interpret the 
data held by the device picked at.


Describe ways that you can overcome these attacks – in what ways can you mitigate these issues?

Attacks can be overcome by placing the system to a more secure network, such as a VPN network, or even adding 
protection to decrease the hacker’s ability to get into the system’s network. Adding content filtering can also be used to 
deal with overwhelming traffic and bad requests. Other ways to overcome attacks include building firewalls, and identifying all users,
as well as encrypting data so that the data ‘stolen’ cannot be easily understood by the attacker, which could persuade them to
potentially move on to easier networks to attack. 

## Sketches and Photos
Sketch the overall flow of information to drive a car with remote control over the Internet

## Modules, Tools, Source Used Including Attribution
Security 


## Supporting Artifacts and References

https://phoenixnap.com/blog/prevent-ddos-attacks


https://www.investopedia.com/terms/e/eavesdropping-attack.asp

https://www.jolera.com/4-things-you-can-do-to-protect-your-organization-from-endpoint-attacks/

-----
