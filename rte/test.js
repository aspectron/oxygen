pragma("event-queue");

var log = require("log");

var oxygen = require("oxygen");

var screen_rect = oxygen.get_screen_rect();
log.info("Screen size: " + screen_rect.width + "x" + screen_rect.height);

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

window.on('resize', function(size)
  { log.info("width: " + size.width + " height: " + size.height);});

window.on('close', function()
  { log.info("window closed");});

window.on('mousemove', function(e) { log.info(e);});
window.on('mousedown', function(e) { log.info(e);});
window.on('mouseup', function(e) { log.info(e);});
window.on('mousewheel', function(e) { log.info(e);});
window.on('mouseclick', function(e) { log.info(e);});
window.on('keydown', function(e) { log.info(e); if (e.vk_code == oxygen.keys.F11) window.toggle_fullscreen(); });
window.on('keyup', function(e) { log.info(e);});
window.on('char', function(e) { log.info(e);});

/*
dpc(25000, function(){
	log.info("Destroying window...");
	window.destroy();
})
*/
