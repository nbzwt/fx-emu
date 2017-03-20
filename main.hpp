
/*
fx-emu -- an ePS6800-based Calculator Emulator
by Wenting Zhang <zephray@outlook.com>
Released under GNU GPLv3
IIII?IIIIIIII+7~??????+=+77777777  7    77 7 7                                  
IIIII====++II77=??I~=====~I=777777777777777,7?=+ 777                           
IIIII?==:===~77I?+====~7777777777777777777I.  777777+I                         
III?==????+=:I?I?==:777777=7777777777777777.777777777777=7                    
=~~~~~~~~~~~~~~~~I77777III7 777777777777777777777777777777+,             
~~~:.,,,..,,,,,,,:~77IIIII7?=,.,:~~~~~~~~:,:+77777777~77777.7:             
~IIIIIIIII===+?+=:~~=II7777~,,:~~~~~~~~~~~~~~~~~~~~=77777777777?77777777 +7     
77?,~=======7IIII==I=7I777777IIII:~~~~~=++=~~~~~~~~~~~~=77777777~777777I~7~     
IIIIII?+===?~~:I?==+I=IIIIIIII+.~~~~~~~~~~~~~~~~=?+~~~~~~~:?7777 :7+=+??=7      
IIIIII====,~~~~:====I=IIIIII.,~~~~~~~~~~~~~~:~~~~~~~~=?+~~~~~,7777:+???I7~      
IIII=====:~~~~~I====II?IIII7~~~:,,,::,,:~~~.,~~~~~~~~~~,~?+?~~~+777==II+77      
II?=====~~~~~~II====:,+IIII+,,~~~~,,,:~~~,7:,~:~~~~~~~~,,~~~?+?~=:7,=?+7      
IIIIIII~~~~~~II=:,,~~~:+III,~:,,.==.:~~:777~,~,~~~~~~~~,,,~~~~=+?+:==:7       
IIIII=~~~~~~~.,.,,~~~~~~~=I,.+.....~~?77I77+,~.,~~~~~~~=,,~~~~~~=??+I          
I?===:~~~~~:,,,,,:~~~~~~~~~.:+.,,.~?,I77777I,~,,~~~~~~~..,~~~~~~~~?+7       
7777:~~~~~,,:7?+?77777+:~:?I.,,,,,I:7?:77777,~,,~~~~~=7=.,,~~,~~~~~?77         
  77~~~~~,I7?III7I7777777I77=~,,,,I.77777777,~,,~~~~7II?.,~::,~~~~~?~~~~~?=   
  7:~~~~~,7??I777~I7II7:777I++++++=77I777777+I.,~~7I?+:?.:~:,,~~~~?++,,,,~~~~   
   ~~,~~.+I=~=~77~7I77777IIIIII:=.7I7777777777,:7I~,=?=:,~~,,:~,~~?~~+7  ~,:~~
  ?~,:~I77.~???+==.===I,IIIIIIII7777777777777777~,,,,,+?.~+,,:,~~?~~~~++     ~,
  +::,77I77?==~,.,,~~~~,I7II7777777777777777777==,,,:7+?.7:,,,~+~~~~~~~~+
  I,I7I777I7=:,,.,,~~~~:777777I:=====~77I777777~++===.7+?=.,,,,~~~,~~~~~~+
  7I77777777+,,,.,,:~~~:777777~========+777777777,=~I7+777?,,,,~~~~~~~~~~~=  
 I77777777I=,,.,,.,,~~~:777777I???????===~+III7IIIIIIIIII~.,,,,~~~~,~~~~~~~~   
7777777777~,,,,.,,,,,~~~=I77I7~?????????=::7777IIIIIIIII:,.,,,,,~,~~~~~~~~~~~
 77777777777:,,,,,,.,,~~:,:I777II???????+?777777IIIIIII,,,,,,,,,,~,~,~~~~,~~~:
  7I77777777IIII.,,,,,,:~.,,.I7I7777II7777777777777II7.,,,,.,,,,,,,,~~:~~ ,:~~ 
     +7777777777777====,,:~,===~+7II77777777777777777,,,,,,,,,,,,,,,,~,~~  ,~~ 
       7+7I777777777:.?7??~:=:======..,~+++++=~,..,,,,,,,,,,~,,.,,,,,:,:~  ,:~ 
          7II777777?::,,?7????=???=:~~:,,,,,,,,?777I:.,,,,,,~~,..,,,,,,,?  ?,:
            77======,::::,?I???+777=??+~~~?:,.77II+7777I?,,~~~:.~,,,,,,     ,
               777~=?+:::::::?I?77+????I??,~,777I77I~7777~~~~,,~: ,,.:        
                  7+?777+,::::,+?+?7I?~,:::=77~=~===I7~=I=7=,,=   ,            
                  7~7777777777?==,I:::::::~+7=77++~~:::~?7+           
                  7?777777777:,:+:,::77777~777+7 :~=++=~~~                     
                  7777777777,::::I::::~:::::7::?I77====~                     
                  77777?777I::::::7+::=:=:7::7?7777II7~                        
*/
#pragma once

#include <iostream>
#include <string>

const std::string VERSION = "0.1.0";

void log(std::string text);