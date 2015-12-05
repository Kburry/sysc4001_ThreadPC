The following program was created for Carleton University's SYSC 4001 Operating Systems course (Fall 2015).
It was written by Reid Cain-Mondoux and Noah Segal via peer programming.
This program emulates a CPU with a number of threads/processes. Each process has a priority level. The CPU is responsible
for scheduling the processes so that they complete in a reasonable amount of time (based off of their priority level).

Reid Cain-Mondoux: 100945700  Noah Segal: 100911661

Supported Platform: Fedora (tested on CB-5109 Lab Computers)


This program must be run via a Unix Terminal*:

1.0 Perform a "make clean" followed by a "make" in the terminal (T):
      T$ make clean
      T$ make

1.1 Run the main file:
      T$ ./main

1.2 Observe the output

1.3 Mark our assignment (hopefully with some generosity)!

1.4 Enjoy your Winter Break!

*NOTE: If you want a less verbose version, please comment out line #136 in main.c
      This line was added as it was a requirement from the Professor. However, it produces a terribly
      large amount of output.



                      _    _  __  _  _ _____  ___ ___
                      | /\ | |__| |\ |   |   |___ |  \
                      |/  \| |  | | \|   |   |___ |__/

         F O R   C R I M E S   A G A I N S T   T H E   E M P I R E
 ________________________  _________________________  _______________________
|        .......         ||      .x%%%%%%x.         ||  ,.------;:~~:-.      |
|      ::::::;;::.       ||     ,%%%%%%%%%%%        || /:.\`;;|||;:/;;:\     |
|    .::;::::;::::.      ||    ,%%%'  )'  \%        ||:')\`:\||::/.-_':/)    |
|   .::::::::::::::      ||   ,%x%) __   _ Y        ||`:`\\\ ;'||'.''/,.:\   |
|   ::`_```_```;:::.     ||   :%%% ~=-. <=~|        ||==`;.:`|;::/'/./';;=   |
|   ::=-) :=-`  ::::     ||   :%%::. .:,\  |        ||:-/-%%% | |%%%;;_- _:  |
| `::|  / :     `:::     ||   `;%:`\. `-' .'        ||=// %wm)..(mw%`_ :`:\  |
|   '|  `~'     ;:::     ||    ``x`. -===-;         ||;;--', /88\ -,- :-~~|  |
|    :-:==-.   / :'      ||     / `:`.__.;          ||-;~~::'`~^~:`::`/`-=:) |
|    `. _    .'.d8:      ||  .d8b.  :: ..`.         ||(;':)%%%' `%%%.`:``:)\ |
| _.  |88bood88888._     || d88888b.  '  /8         ||(\ %%%/dV##Vb`%%%%:`-. |
|~  `-+8888888888P  `-. _||d888888888b. ( 8b       /|| |);/( ;~~~~ :)\`;;.``\|
|-'     ~~^^^^~~  `./8 ~ ||~   ~`888888b  `8b     /:|| //\'/,/|;;|:(: |.|\;|\|
|8b /  /  |   \  \  `8   ||  ' ' `888888   `8. _ /:/||/) |(/ | / \|\\`( )- ` |
|P        `          8   ||'      )88888b   8b |):X ||;):):)/.):|/) (`:`\\`-`|
|                    8b  ||   ~ - |888888   `8b/:/:\||;%/ //;/(\`.':| ::`\\;`|
|                    `8  ||       |888888    88\/~~;||;/~( \|./;)|.|):;\. \\-|
|                     8b ||       (888888b   88|  / ||/',:\//) ||`.|| (:\)):%|
| LS      .           `8 ||\       \888888   8-:   /||,|/;/(%;.||| (|(\:- ; :|
|________/_\___________8_||_\_______\88888_.'___\__/||_%__%:__;_:`_;_:_.\%_`_|

L u k e  S k y w a l k e r      H a n   S o l o          C h e w b a c c a

Self-Proclaimed Jedi Knight     Smuggler, Pirate         Smuggler, Pirate
     500,000 credits            200,000 Credits          100,000 credits

               The above are wanted for the following crimes:

    - Liberation of a known criminal, Princess Leia Organa of Alderaan -
         - Direct involvement in armed revolt against the Empire -
                              - High treason -
                               - Espionage -
                               - Conspiracy -
                    - Destruction of Imperial Property -

           These individuals are considered extremely dangerous.

       E X P E R I E N C E D   B O U N T Y   H U N T E R S   O N L Y

  The Empire will not  be held  responsible  for any  injuries or property
  loss arising from the  attempted apprehension of these  notorious crimi-
  nals. Bounty is for live capture only! For more information contact your
  local Imperial Intelligence Office.
  
  http://the.sunnyspot.org/asciiart/gallery/starwars.html
