exports.convert = function (file) {
    var lines = file.split('\n').map(trim);
    var names = /^o .*$/;
    var vertices = /^v (.*) (.*) (.*)$/;
    var normals = /^vn (.*) (.*) (.*)$/;
    var faces = /^f (.*)\/\/(.*) (.*)\/\/(.*) (.*)\/\/(.*)$/;
    
    return filterMatches(names)
        .concat(['GLfloat v[][3] = {'])
        .concat(filterMatches(vertices))
        .concat(['};', '', 'GLfloat vn[][3] = {'])
        .concat(filterMatches(normals))
        .concat(['};', ''])
        .concat(filterMatches(faces))
        .concat(['}'])
        .map(convertLine);

    function trim(x) {
        return x.trim();
    }

    function filterMatches(re) {
        return lines.filter(function (line) {
            return re.test(line)
        });
    }
};

function convertLine(line) {
    if (/^o .*$/.test(line)) {
        return 'static void ' + line.substr(2) + '(void) {';
    }

    var match = /^vn? (.*) (.*) (.*)$/.exec(line);

    if (match) {
        return '{' + match[1] + ', ' + match[2] + ', ' + match[3] + '},';
    }

    if (/^f /.test(line)) {
        return createFace();
    }

    return line;

    function createFace() {
        var r = 'nvs(vn, v, (int [][2]) {';
        var fn = /(\d+)\/\/(\d+)/g;
        var v;
        var x = [];

        while (v = fn.exec(line)) {
            x.push('{' + v[1] + ', ' + v[2] + '}');
        }

        r += x.join(', ');
        r += '}, ';
        r += x.length;
        r += ');';
        return r;
    }
}
