var peerfields = [
    {name : 'addr', type : 'string'},
    {name : 'services', type : 'string'},
    {name : 'lastsend', type : 'int'},
    {name : 'lastrecv', type : 'int'},
    {name : 'bytessent', type : 'int'},
    {name : 'bytesrecv', type : 'int'},
    {name : 'conntime', type : 'int'},
    {name : 'version', type : 'int'},

    {name : 'subver', type : 'string'},
    {name : 'inbound', type : 'boolean'},
    {name : 'releasetime', type : 'int'},
    {name : 'startingheight', type : 'int'},
    {name : 'banscore', type : 'int'},
];

function timestampRenderer(value)
{
    var date = Ext.Date.parse(value, 'U');
    return Ext.Date.format(date, 'd.m.Y, H:i:s');
}

function LoadPeerInfo()
{
    var req = {
      id: request_id++,
      jsonrpc: '2.0',
      method: 'getpeerinfo',
      params: []
    };

    var reqResult = null;

    Ext.Ajax.request({
        url : '/',
        method: 'POST',
        jsonData: Ext.encode(req),

        success: function(response) {
            var data = Ext.decode(response.responseText);

            if (data.result != null)
                Ext.getCmp('peergrid').getStore().loadData(data.result);
            else
                Ext.Msg.alert('Error', data.error.message);
        },

        failure: function(response) {
            Ext.Msg.alert('Error', response.responseText);
        }
    });
}

var peergrid = new Ext.create('Ext.grid.Panel', {
    id: 'peergrid',
    title: 'Peer info',
    store: new Ext.data.JsonStore({fields: peerfields, data: []}),
    columns: [
        {header: "Address", width: 100, dataIndex: 'addr', sortable: true},
        {header: "Services", dataIndex: 'services', sortable: true},
        {header: "Last send", dataIndex: 'lastsend', renderer: timestampRenderer, sortable: true},
        {header: "Last receive", dataIndex: 'lastrecv', renderer: timestampRenderer, sortable: true},
        {header: "Bytes sent", dataIndex: 'bytessent', sortable: true},
        {header: "Bytes received", dataIndex: 'bytesrecv', sortable: true},
        {header: "Connection time", dataIndex: 'conntime', renderer: timestampRenderer, sortable: true},
        {header: "Version", dataIndex: 'version', sortable: true},

        {header: "Subver", width: 100, dataIndex: 'subver', renderer: Ext.util.Format.htmlEncode, sortable: true},
        {header: "Inbound", dataIndex: 'inbound', sortable: true, xtype: 'checkcolumn', processEvent: Ext.emptyFn},
        {header: "Release time", dataIndex: 'releasetime', sortable: true},
        {header: "Starting height", dataIndex: 'startingheight', sortable: true},
        {header: "Ban score", dataIndex: 'banscore', sortable: true}
    ],
    listeners: {
        activate: LoadPeerInfo
    },
    buttons: [
        {
            xtype: 'button',
            text: 'Refresh',
            handler: LoadPeerInfo
        },
    ],
    layout: 'fit'
});
