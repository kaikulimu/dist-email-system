# Distributed Email System
A fault-tolerant distributed email system using Anti-Entropy method with eventual path propagation. There are _N_ server replicas, with _N_=5 by default. Note that a [Spread](http://spread.org/) daemon must be run at the same host where a server is run.

The mail server is started with its id as a parameter:
    
    $ ./server [1-5]
    
The mail client can be run with simply:
    
    $ ./client

## Client functionalities
The email system provides the following capabilities; each capability is one option of a client program's top level menu:
1. "u _\<user name\>_": Login with _\<user name\>_. 
2. "c _\<id\>_": Connect to Server _\<id\>_. The client needs to connect to a mail server to perform any of the tasks below.
3. "l": List the headers of received mail.
4. "m": Mail a message to a user. Subsequently, enter:  
&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;to:  _\<user name\>_  
&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;subject:  _\<subject string\>_  
&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;_\<msg string\>_
5. "r _i_": Read the _i_-th message in the current list of messages.
6. "d _i_": Delete the _i_-th message in the current list of messages.
7. "v": Print the membership (identities) of the mail servers in the current mail server's network component. 
