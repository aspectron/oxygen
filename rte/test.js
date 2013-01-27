SET_EXECUTION_OPTIONS(EVENT_QUEUE | NO_GC_NOTIFY);

var log = require("log");

var oxygen = require("oxygen");


log.info("Creating window...");

var window = oxygen.window({
	width: 640,
	height: 480,
	bpp: 32,
	caption: "oxygen",
//	frame: true,
	style : oxygen.styles.APPLICATION
})

log.info("Window created...");

dpc(25000, function(){
	log.info("Destroying window...");
	window.destroy();
})
