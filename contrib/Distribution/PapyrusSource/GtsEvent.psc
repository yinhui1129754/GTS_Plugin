scriptName GtsEvent hidden

; OnFootstep event
;
; These will fire a callback of the form
;
; Event OnFootstep(Actor actor, String tag)
; EndEvent
;
; When a footstep event is detected
;
;
; Registeration must done on every startup with:
; RegisterOnFootstep(a_form)
; This form is usually self if registering for the event in the
; target script
Function RegisterOnFootstep(Form form) global native
Function UnRegisterOnFootstep(Form form) global native
