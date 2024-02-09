This is a small, hacky tool to reset the VMM7100 USB USB-C to HDMI 
adapter.  It is the result of analyzing the messages sent by VMMHIDTool 
under windows.

I know very little about OSX USB programming so this is extremely hacky 
and based on some sample code I've seen, but it seems to work. 

Note that after resetting the adapter(s), there must be some event to get 
OSX to notice the adapters and wake  the screens up.  So resetting the 
adapter, THEN powering up the monitors usually does the trick.   

There is a swift rewrite of this availble here:

https://github.com/waydabber/vmm7100reset/tree/main
