pragma("event-queue");

var oxygen = require("oxygen");

console.log("Screen size:", oxygen.getScreenSize());

console.log("Creating window...");

var window = oxygen.Window({
	width: 640,
	height: 480,
	bpp: 32,
	caption: "oxygen",
	style : oxygen.styles.APPLICATION
})

console.log("Window created...");

window.on('resize', function(size) { console.log("resize:", size); });
window.on('close', function()  { console.log("window closed"); });
window.on('mousemove', function(e) { console.log(e); });
window.on('mousedown', function(e) { console.log(e); });
window.on('mouseup', function(e) { console.log(e); });
window.on('mousewheel', function(e) { console.log(e); });
window.on('mouseclick', function(e) { console.log(e); });
window.on('keydown', function(e) { console.log(e); if (e.vk_code == oxygen.keys.F11) window.toggle_fullscreen(); });
window.on('keyup', function(e) { console.log(e); });
window.on('char', function(e) { console.log(e); });

/*
dpc(25000, function(){
	console.log("Destroying window...");
	window.destroy();
})
*/
