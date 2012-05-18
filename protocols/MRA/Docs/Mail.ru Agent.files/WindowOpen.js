//
// Proxomitron Anti-popup code
//

var PrxLC=new Date(0);
var PrxModAtr=0;
var PrxLd=1;
var PrxInst; if(!PrxInst++) PrxRealOpen=window.open;

function PrxOMUp(){PrxLC=new Date();}
function PrxNW(){return(this.window);} 
function PrxOpen(url,nam,atr){
 if(PrxLC){
  var cdt=new Date();
  cdt.setTime(cdt.getTime()-PrxLC.getTime());
  if(!PrxLd && cdt.getSeconds()<2){
    if(atr){ return(PrxRealOpen(url,nam,PrxWOA(atr))); }
    else{ return(PrxRealOpen(url,nam)); }
  }
 }
 return(new PrxNW());
} 

function PrxWOA(atr){
  var xatr="location=yes,status=yes,resizable=yes,toolbar=yes,scrollbars=yes";
  if(!PrxModAtr) return(atr);
  if(atr){
    var hm;
    hm=atr.match(/height\=[0-9]+/i);
    if(hm) xatr+="," + hm;
    hm=atr.match(/width\=[0-9]+/i);
    if(hm) xatr+="," + hm;
  }
  return(xatr);
}

function PrxRST(){
  if(document.layers){document.captureEvents(Event.MOUSEUP);}
  document.onmouseup=PrxOMUp;
  PrxLd=0;
}

window.open=PrxOpen;
submit=PrxOpen;