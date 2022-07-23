/*
2022-07-23
Notes: pin labeling did not print correctly so they have been commented out. The wire path intended for strain relief did not seem big enough for both wires and probably should be enlarged.

2022-07-02
This OpenSCAD design allows for OpenVTX bootloader programming onto the EWRF E7082VM board without needing to solder leads onto the CLK or DIO pads. You will still need to connect power and a common ground to the ST-LINK V2.  I have verified this print allowed me to flash OpenVTX bootloader to my VTX labeled 7082VM V12. I printed this in TPU/TPE and needed to re-poke holes for the wires with a sim card eject tool. I then ran some solid hook up wire as shown in images taking care that the wires do not short. In order to make good contact with the CLK and DIO pads I do recomend utilizing a sharp hobby knife to carefully shave the excess perferations from the side of the PCB until smooth.

*/
$fn=90;

BoardWidth=14;
BoardLength=15;
BoardThick=0.8;
PadRad=0.9*0.5; // actual diameter 0.6mm is a bit small for most printers
PadPostRad=1.8*0.5;
PadAWidth=1;
PadALength=5.8;
PadBWidth=2;
PadBLength=4.8;
HolderThick=3;
HolderPadding=2;
HolderBaseOverhang=3;
HolderMaxOverhang=6;
HolderMaxOverhangLength=(PadALength-PadBLength)*0.5+PadBLength;
HolderRad=5*0.5;
ConduitRad=2.6*0.5;
TextSize=2.1;


rotate([0,180,0]) EWRF_E7082VM_OpenVTX_bootloader_tool();

//
module EWRF_E7082VM_OpenVTX_bootloader_tool(){
difference(){
hull(){
// first corner
translate([0,0,(HolderThick+HolderPadding)*0.5]) cylinder(HolderThick+HolderPadding,HolderPadding,HolderPadding,true);
// far corner
translate([0,BoardLength,(HolderThick+HolderPadding)*0.5]) cylinder(HolderThick+HolderPadding,HolderPadding,HolderPadding,true);
// first corner
translate([HolderPadding,0,(HolderThick+HolderPadding)*0.5]) cylinder(HolderThick+HolderPadding,HolderPadding,HolderPadding,true);
// far corner
translate([HolderPadding,BoardLength,(HolderThick+HolderPadding)*0.5]) cylinder(HolderThick+HolderPadding,HolderPadding,HolderPadding,true);    
// thumb tab
translate([HolderMaxOverhang-HolderRad,HolderMaxOverhangLength,(HolderThick+HolderPadding)*0.5]) cylinder(HolderThick+HolderPadding,HolderRad,HolderRad,true);
// Conduit
translate([HolderBaseOverhang*0.25,-HolderPadding,(HolderThick+HolderPadding)*0.5]) cylinder(HolderThick+HolderPadding,ConduitRad+HolderPadding*0.5,ConduitRad+HolderPadding*0.5,true);
}
// begin main difference items
// main board cavity
difference(){
translate([0,0,-0.02]) cube([BoardWidth,BoardLength,HolderThick]);
hull(){
// PadA Post
translate([PadAWidth,PadALength,(HolderThick+HolderPadding)*1-BoardThick]) cylinder(HolderThick+HolderPadding+0.04,PadPostRad,PadPostRad,true);
// PadB Post
translate([PadBWidth,PadBLength,(HolderThick+HolderPadding)*1-BoardThick]) cylinder(HolderThick+HolderPadding+0.04,PadPostRad,PadPostRad,true);
}
}    
// PadA cavity
translate([PadAWidth,PadALength,(HolderThick+HolderPadding)*0.5]) cylinder(HolderThick+HolderPadding+0.04,PadRad,PadRad,true);
// PadB cavity
translate([PadBWidth,PadBLength,(HolderThick+HolderPadding)*0.5]) cylinder(HolderThick+HolderPadding+0.04,PadRad,PadRad,true);
//conduit cavity
translate([HolderBaseOverhang*0.25,-HolderPadding,(HolderThick+HolderPadding)*0.5]) cylinder(HolderThick+HolderPadding+0.04,ConduitRad,ConduitRad,true);
// text label DIO
//translate([-1.6,PadALength+1,HolderThick+HolderPadding-0.8]) linear_extrude(1) text("DIO", size = TextSize);
// text label CLK
//translate([-1.6,PadBLength-2.8,HolderThick+HolderPadding-0.8]) linear_extrude(1) text("CLK", size = TextSize);
}
}