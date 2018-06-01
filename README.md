# phenixdaq_study (mchiu)
Monte Carlo of PHENIX Style DAQ

This is a monte carlo to study dynamic effects on the livetime of the PHENIX DAQ.
The code implements the following features of the PHENIX DAQ and collider structure:

deadfor       - The number of clocks after a trigger where the DAQ is dead
convert time  - Time in clocks it takes the FEM to convert data for sending (dominated by digitization time)
endat time    - Time in clocks for FEM to send data (ENDAT = enable data send)
countN        - number of triggers to buffer at a time

abort gap     - crossings 110-119 are always empty
dead crossing - crossings which are full, but which we deadtime out (excludes abort gap)
              - examples include the GL1 reset, VTXS reset
PCFEM unrel   - PC FEM reset. Unlike a dead crossing, this reset doesn't occur every revolution.

You have to modify phenixdaqrate.C if you want to turn on/off dead crossings and the PCFEM unrel.
By default there are no dead crossings, and the pcfem unreliable is turned off, which is what one
would want for sPHENIX studies. 

To run the code (which is just a simple ROOT macro), just type at the root command line

 .L phenixdaqrate.C+
 phenixdaqrate(400,860,4,20000000);

The above simulates a Run14AuAu run, where the slowest system is the VTXS (400 = CONV, 860 = 2*430 ENDAT),
and COUNTN = 4 specifies the max number of buffered triggers.  The ENDAT is x2 because they have 2 ENDATs.
The 20000000 means you simulate 20 million collisions. You can make this as high as you want (up to the
integer max).

The code generates a "daqrate.root" file which contains two livetime vs rate TGraphs. One is called
"model" (for the livetime vs raw rate), and the other is called "modelreal" (livetime vs accepted
event rate).  "modelreal" is what is really used by people, since the accepted event rate is what we 
normally call the daq rate.  Sorry for the stupid naming convention - it's a historical relic now.

Collisions are generated randomly in every crossing except the abort gap, and then simulates the
busy state of the DAQ to determine whether the event is accepted.  

