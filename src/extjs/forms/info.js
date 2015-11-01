Ext.define('InfoRecord', {
    extend: 'Ext.data.Model',
    fields: [
        {name : 'version', type : 'string'},
        {name : 'protocolversion', type : 'int'},
        {name : 'walletversion', type : 'int'},
        {name : 'balance', type : 'float'},
        {name : 'unspendable', type : 'float'},
        {name : 'newmint', type : 'float'},
        {name : 'stake', type : 'float'},
        {name : 'blocks', type : 'int'},
        {name : 'systemclock', type: 'date', dateFormat: 'timestamp'},
        {name : 'adjustedtime', type: 'date', dateFormat: 'timestamp'},
        {name : 'ntpoffset', type : 'int'},
        {name : 'p2poffset', type : 'int'},
        {name : 'proof-of-stake', type : 'float'},
        {name : 'proof-of-work', type : 'float'},
        {name : 'moneysupply', type : 'float'},
        {name : 'connections', type : 'int'},
        {name : 'proxy', type : 'string'},
        {name : 'ip', type : 'string'},
        {name : 'testnet', type : 'boolean'},
        {name : 'keypoololdest', type : 'int'},
        {name : 'keypoolsize', type : 'int'},
        {name : 'paytxfee', type : 'float'},
        {name : 'mininput', type : 'float'},
    ]
});

var loadInfo = function() {
    Ext.Ajax.request({
        url : '/',
        async: false,
        method: 'POST',
        jsonData: Ext.encode({
            id: request_id++,
            jsonrpc: '2.0',
            method: 'getinfo',
            params: []
        }),
        success: function(response) {
            var data = Ext.decode(response.responseText);

            if (data.result != null)
            {
                var info = data.result;
                Ext.apply(info, info.difficulty);
                Ext.apply(info, info.timestamping);
                delete info.difficulty;
                delete info.timestamping;
                var record = Ext.create('InfoRecord', info);

                Ext.getCmp('getinfo').getForm().loadRecord(record);
            }
            else
                Ext.Msg.alert('Error', data.error.message);
        },
        failure: function(response) {
            Ext.Msg.alert('Error', response.responseText);
        }
    });
};

var InfoForm = Ext.create('Ext.form.Panel', {
    id: 'getinfo',
    region: 'center',
    title: 'Basic info',
    fieldDefaults: {
        msgTarget: 'side'
    },
    defaults: {
        anchor: '50%'
    },
    defaultType: 'textfield',
    bodyPadding: '5 5 0',
    items: [
        {fieldLabel: 'Version', htmlEncode: true, name: 'version', disabled: true},
        {fieldLabel: 'Protocol version', name: 'protocolversion', disabled: true},
        {fieldLabel: 'Wallet version', name: 'walletversion', disabled: true},
        {fieldLabel: 'Balance', name: 'balance', disabled: true},
        {fieldLabel: 'Unspendable', name: 'unspendable', disabled: true},
        {fieldLabel: 'New mint', name: 'newmint', disabled: true},
        {fieldLabel: 'Stake', name: 'stake', disabled: true},
        {fieldLabel: 'Blocks', name: 'blocks', disabled: true},
        {fieldLabel: 'System clock', name: 'systemclock', disabled: true},
        {fieldLabel: 'Adjusted time', name: 'adjustedtime', disabled: true},
        {fieldLabel: 'NTP offset', name: 'ntpoffset', disabled: true},
        {fieldLabel: 'P2P offset', name: 'p2poffset', disabled: true},
        {fieldLabel: 'Stake diff', name: 'proof-of-stake', disabled: true},
        {fieldLabel: 'Work diff', name: 'proof-of-work', disabled: true},
        {fieldLabel: 'Money supply', name: 'moneysupply', disabled: true},
        {fieldLabel: 'Connections', name: 'connections', disabled: true},
        {fieldLabel: 'Proxy', name: 'proxy', disabled: true},
        {fieldLabel: 'IP', name: 'ip', disabled: true},
        {
            fieldLabel: 'Testnet',
            name: 'testnet',
            xtype: 'checkboxfield',
            checked: false,
            disabled: true,
            value: false
        },
        {fieldLabel: 'Custom fee', name: 'paytxfee', disabled: true},
        {fieldLabel: 'Minimal value', name: 'mininput', disabled: true}
    ],
    listeners: {
        activate: loadInfo
    },
    buttons: [
        {
            xtype: 'button',
            text: 'Refresh',
            handler: loadInfo
        },
    ]
});
