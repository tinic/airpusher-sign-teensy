#target photoshop
s2t = stringIDToTypeID;

(r = new ActionReference()).putProperty(s2t('property'), k = s2t('countClass'));
r.putEnumerated(s2t('document'), s2t('ordinal'), s2t('targetEnum'));
var p = executeActionGet(r);
if (p.hasKey(k)) {
    var counter = p.getList(k),
        n = (new File).saveDlg('Save file', '*.csv');
    if (n) {
        if (n.open('w', 'TEXT')) {
            n.write('i;g;x;y\n')
            for (var i = 0; i < counter.count; i++) {
                var c = counter.getObjectValue(i)
                n.write(c.getDouble(s2t('group')) + ', ' + c.getDouble(s2t('itemIndex')) + ', ' + c.getDouble(s2t('x')) + ', ' + c.getDouble(s2t('y')) + ',\n')
            }
            n.close()
        }
    }
}
