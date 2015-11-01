var addressfields = [
    {name : 'address', type : 'string'},
    {name : 'account', type : 'string'},
];

function LoadAddressInfo()
{
    var req = {
      id: request_id++,
      jsonrpc: '2.0',
      method: 'getaddresses',
      params: []
    };

    var reqResult = null;

    Ext.Ajax.request({
        url : '/',
        async: false,
        method: 'POST',
        jsonData: Ext.encode(req),

        success: function(response) {
            var data = Ext.decode(response.responseText);

            if (data.result != null)
                Ext.getCmp('addressesgrid').getStore().loadData(data.result);
            else
                Ext.Msg.alert('Error', data.error.message);
        },

        failure: function(response) {
            Ext.Msg.alert('Error', response.responseText);
        }
    });
}

var addressesgrid = new Ext.create('Ext.grid.Panel', {
    id: 'addressesgrid',
    title: 'Addresses',
    store: new Ext.data.JsonStore({fields: addressfields, groupField: 'account', groupDir: 'ASC', data: []}),
    columns: [
        {header: "Address", dataIndex: 'address', width: 300, sortable: true},
        {header: "Account", dataIndex: 'account', sortable: false}
    ],
    features: [{
        ftype: 'grouping',
        groupHeaderTpl: '{name} ({children.length})',
        enableNoGroups: true
    }],
    listeners: {
        activate: LoadAddressInfo
    },
    buttons: [
        {
            xtype: 'button',
            text: 'Refresh',
            handler: LoadAddressInfo
        },
    ],
    layout: 'fit'
});
