scriptName GtsHeight hidden

; Target Height
;
; These will be the target height which is achieved on the main loop over the
; next few frames gradually
; Use this to alter the current height
;
; It works using the GtsScale.SetScale method
;   You can adjust which scale is adjusted using GtsScale.SetScaleMethod
;
; Target height is saved into the COSAVE and will persist
function SetTargetHeight(Actor target, Float height) global native

Float function GetTargetHeight(Actor target) global native

function ModTargetHeight(Actor target, Float amount) global native

; Max Height
;
; These will set the max height
;
; Max height is saved into the COSAVE and will persist
function SetMaxHeight(Actor target, Float height) global native

Float function GetMaxHeight(Actor target) global native

function ModMaxHeight(Actor target, Float amount) global native

; Visual Height
;
; This is the current actual height of the actor. While the target height
; is what this value aims for, before it gets to the target height
; then this represents the actual height
;
; Use this for any size effects
;
; Plan is to have growth be stop by obstacles to the target height
; like ceilings, and this will be the actual height that is achieved.
;
; Visual height is saved into the COSAVE and will persist
Float function GetVisualHeight(Actor target) global native


; TeamMate height mod
;
; This will mod the target height of all teammates nearby
function ModTeammateHeight(Float amount) global native
