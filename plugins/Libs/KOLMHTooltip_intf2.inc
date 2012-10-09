// part of KOLMHToolTip -- interface_part.
// Moved to separate inc-file still Delphi20XX does not allow compile 
// in DEBUG mode.
const
  Dummy = 0;


function NewHint(A: PControl): PMHHint;
function NewManager: PMHToolTipManager;
function NewMHToolTip(AParent: PControl): PMHToolTip;

var
  Manager: PMHToolTipManager;
