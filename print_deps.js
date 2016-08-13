var path = require('path')

var image_module = require.resolve('image')

function replace_ext(filename, new_ext)
{
	return path.join(
		path.dirname(filename),
		path.basename(filename, path.extname(filename)) + new_ext)
}

switch (process.argv[2])
{
case 'include':
	image_dir = path.join(image_module, '../../../include')
	console.log(image_dir)
	break
case 'lib':
	image_lib = image_module
	if (process.platform === 'win32')
	{
		image_lib = replace_ext(image_lib, '.lib')
	}
	console.log(image_lib)
	break
}