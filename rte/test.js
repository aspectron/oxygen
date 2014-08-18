pragma("event-queue");

var oxygen = require("oxygen");


oxygen.Display.enumerate().forEach(function(display)
	{
		console.log("display name: %s colorDepth: %d rectangle: %o workRectangle: %o",
			display.name, display.colorDepth, display.rectangle, display.workRectangle);
		console.log('display modes:', display.modes());
		console.log('display current mode:', display.currentMode());
	});

console.log('primary display:', oxygen.Display.primary().name)

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

console.log(window.runFileDialog({
	type: 'save',
	title: 'My title',
	multiselect: true,
	defaultDir: 'D:\\',
	defaultName: 'filename',
	defaultExt: '.xml',
	filter: {
		'*.pdf': 'PDF documents',
		'*.html': 'HTML documents',
		'*.rtf': 'RTF documents',
		'*.doc;*.docx': 'Microsoft Word documents',
		'*.pdf;*.html;*.rtf;*doc;*.docx': 'All supported documents',
		'*.*': 'All files',
	},
}));