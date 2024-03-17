scriptName GtsScale hidden

; Model scale
;
; These directly and immediatly set the scale
Bool function SetModelScale(Actor target, Float scale) global native

Float function GetModelScale(Actor target) global native

Bool function ModModelScale(Actor target, Float amount) global native

; Npc Node scale
;
; These directly and immediatly set the scale of the NPC Root node
Bool function SetNodeScale(Actor target, Float scale) global native

Float function GetNodeScale(Actor target) global native

Bool function ModNodeScale(Actor target, Float amount) global native

; Hybrid scale
;
; These directly and immediatly set the scale of the form
; They work differentlt on the player than anyone else
Bool function SetHybridScale(Actor target, Float scale) global native

Float function GetHybridScale(Actor target) global native

Bool function ModHybridScale(Actor target, Float amount) global native

; All scale
;
; These directly and immediatly set the combined scale
;
; Get scale works as:
;   GetRefScale * GetNodeScale * GetModelScale
;
; Set scale works by making
;   GetRefScale * GetNodeScale * GetModelScale = value
;
; To do this it adjusts one of the scales (Model scale by default)
;
; You can control which scale is adjusted using
; SetScaleMethod
Bool function SetScale(Actor target, Float scale) global native

Float function GetScale(Actor target) global native

Bool function ModScale(Actor target, Float amount) global native

; Set the method used in All SetScale
; 0 - Model Scale
; 1 - Npc Node Scale
; 2 - Ref Scale
Function SetScaleMethod(Int method)  global native

Int Function GetScaleMethod()  global native


; Target Scale
;
; These will be the target scale which is achieved on the main loop over the
; next few frames gradually
; Use this to alter the current scale
;
; It works using the SetScale method
;   You can adjust which scale is adjusted using SetScaleMethod
;
; Target scale is saved into the COSAVE and will persist
function SetTargetScale(Actor target, Float scale) global native

Float function GetTargetScale(Actor target) global native

function ModTargetScale(Actor target, Float amount) global native

; Max Scale
;
; These will set the max scales
;
; Max scale is saved into the COSAVE and will persist
function SetMaxScale(Actor target, Float scale) global native

Float function GetMaxScale(Actor target) global native

function ModMaxScale(Actor target, Float amount) global native

; Visual Scale
;
; This is the current actual scale of the actor. While the target scale
; is what this value aims for, before it gets to the target scale
; then this represents the actual scale
;
; Use this for any size effects
;
; Plan is to have growth be stop by obstacles to the target scale
; like ceilings, and this will be the actual scale that is achieved.
;
; Visual scale is saved into the COSAVE and will persist
Float function GetVisualScale(Actor target) global native


; TeamMate scale mod
;
; This will mod the target scale of all teammates nearby
function ModTeammateScale(Float amount) global native

; Report real default scale after all size calculations (RaceMenu, Race Scale)
;
function GetOtherScale(Actor* target) global native

; Report visuals scale without any adjustments, pure scale value
;
float function GetGiantessScale(Actor target) global native
