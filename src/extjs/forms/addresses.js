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

    Ext.Ajax.request({
        url : '/',
        method: 'POST',
        jsonData: Ext.encode(req),

        success: function(response) {
            var data = Ext.decode(response.responseText);

            if (data.error == null)
                Ext.getCmp('addressesgrid').getStore().loadData(data.result);
            else
                Ext.Msg.alert('Error', data.error.message);
        },

        failure: function(response) {
            Ext.Msg.alert('Error', response.responseText);
        }
    });
}

function NewAddress(strAddrLabel)
{
    var req = {
      id: request_id++,
      jsonrpc: '2.0',
      method: 'getnewaddress',
      params: [strAddrLabel]
    };

    Ext.Ajax.request({
        url : '/',
        method: 'POST',
        jsonData: Ext.encode(req),

        success: function(response) {
            var data = Ext.decode(response.responseText);

            if (data.error == null)
                LoadAddressInfo();
            else
                Ext.Msg.alert('Error', data.error.message);
        },

        failure: function(response) {
            Ext.Msg.alert('Error', response.responseText);
        }
    });
}

function SetLabel(strAddress, strLabel)
{
    var req = {
      id: request_id++,
      jsonrpc: '2.0',
      method: 'setaccount',
      params: [strAddress, strLabel]
    };

    Ext.Ajax.request({
        url : '/',
        method: 'POST',
        jsonData: Ext.encode(req),

        success: function(response) {
            var data = Ext.decode(response.responseText);

            if (data.error == null)
                LoadAddressInfo();
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
        {header: "Label", dataIndex: 'account', sortable: false}
    ],
    features: [{
        ftype: 'grouping',
        groupHeaderTpl: '{name} ({children.length})',
        enableNoGroups: true
    }],
    listeners: {
        activate: LoadAddressInfo,
        celldblclick: function(ctx, td, cellIndex, record, tr, rowIndex, e, eOptions)
        {
            // console.log(record);

            Ext.Msg.prompt('New label for ' + record.data.address, 'Please enter new label', function(btn, text)
            {
                if (btn == 'ok')
                {
                    SetLabel(record.data.address, text);
                }
            }, this, false, record.data.account);
        }
    },
    buttons: [
        {
            xtype: 'button',
            text: 'New address',
            handler: function() {
                Ext.Msg.prompt('New address', 'Please enter label for new address', function(btn, text)
                {
                    if (btn == 'ok')
                    {
                        NewAddress(text);
                    }
                });
            }
        },
        {
            xtype: 'button',
            text: 'Refresh',
            handler: LoadAddressInfo
        }
    ],
    layout: 'fit'
});
