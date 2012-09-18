var oxygen = (function()
{
    var bindings = require("bindings");
    var oxygen = new bindings.library("oxygen");

    return oxygen;

})();

exports.$ = oxygen;
