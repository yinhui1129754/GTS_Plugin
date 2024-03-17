ScriptName GtsSM_HotKeys Extends Quest

Actor Property PlayerRef Auto

Function GrowPlayer()
  GtsScale.ModTargetScale(PlayerRef, 0.1)
EndFunction

Function ShrinkPlayer()
  GtsScale.ModTargetScale(PlayerRef, -0.1)
EndFunction

Function TremorScale(Float value)
  GtsPlugin.SetTremorScale(value)
EndFunction

Function SetTremorScaleNPC(Float value)
  GtsPlugin.SetTremorScaleNPC(value)
EndFunction

Function PlayerGrowthHalfLife(Float rate)
  GtsPlugin.SetGrowthHalfLife(PlayerRef, rate)
EndFunction

Function Experiment(Float value)
  GtsPlugin.SetExperimentFloat(value)
EndFunction


Function GrowFollowers()
  ; GtsScale.ModTeammateScale(0.1)
  GtsControl.GrowTeammate(1.0)
EndFunction

Function ShrinkFollowers()
  ;GtsScale.ModTeammateScale(-0.1)
  GtsControl.ShrinkTeammate(1.0)
EndFunction
