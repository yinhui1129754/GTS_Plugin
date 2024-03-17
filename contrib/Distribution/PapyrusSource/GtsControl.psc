scriptName GtsControl hidden

; Grow TeamMate
;
; This is fired with the hotkey
function GrowTeammate(Float power) global native

; Shink TeamMate
;
; This is fired with the hotkey
function ShrinkTeammate(Float power) global native

; Grow / Shrink the Player similar to 2 functions above (but the target is player in this case)
; Fired with the hotkeys

function GrowPlayer(Float power) global native
function ShrinkPlayer(Float power) global native
    
;Rapid player growth
function CallRapidGrowth(float amt, float halflife) global native 
 
;Rapid player shrink
function CallRapidShrink(float amt, float halflife) global native    
