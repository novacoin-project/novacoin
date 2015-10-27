function rpc_getinfo()
{
    var req = {
      id: request_id++,
      jsonrpc: '2.0',
      method: 'getinfo',
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
                reqResult = data.result;
            else
                Ext.Msg.alert('Error', data.error.message);
        },

        failure: function(response) {
            Ext.Msg.alert('Error', response.responseText);
        }
    });

    return reqResult;
}

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

        {name : 'systemclock', type : 'int'},
        {name : 'adjustedtime', type : 'int'},
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
    var formObj = Ext.getCmp('getinfo').getForm();
    var info = rpc_getinfo();

    Ext.apply(info, info.difficulty);
    Ext.apply(info, info.timestamping);

    delete info.difficulty;
    delete info.timestamping;

    var record = Ext.create('InfoRecord', info);

    formObj.loadRecord(record);
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
        {
            fieldLabel: 'Version',
            htmlEncode: true,
            name: 'version',
        },
        {
            fieldLabel: 'Protocol version',
            name: 'protocolversion',
        },
        {
            fieldLabel: 'Wallet version',
            name: 'walletversion',
        },
        {
            fieldLabel: 'Balance',
            name: 'balance',
        },
        {
            fieldLabel: 'Unspendable',
            name: 'unspendable',
        },
        {
            fieldLabel: 'New mint',
            name: 'newmint',
        },
        {
            fieldLabel: 'Stake',
            name: 'stake',
        },
        {
            fieldLabel: 'Blocks',
            name: 'blocks',
        },
        {
            fieldLabel: 'System clock',
            name: 'systemclock',
        },
        {
            fieldLabel: 'Adjusted time',
            name: 'adjustedtime',
        },
        {
            fieldLabel: 'NTP offset',
            name: 'ntpoffset',
        },
        {
            fieldLabel: 'P2P offset',
            name: 'p2poffset',
        },
        {
            fieldLabel: 'Stake diff',
            name: 'proof-of-stake',
        },
        {
            fieldLabel: 'Work diff',
            name: 'proof-of-work',
        },
        {
            fieldLabel: 'Money supply',
            name: 'moneysupply',
        },
        {
            fieldLabel: 'Connections',
            name: 'connections',
        },
        {
            fieldLabel: 'Proxy',
            name: 'proxy',
        },
        {
            fieldLabel: 'IP',
            name: 'ip',
        },
        {
            fieldLabel: 'Testnet',
            name: 'testnet',
            xtype: 'checkboxfield',
            checked: false,
            disabled: true,
            value: false
        },
        {
            fieldLabel: 'Custom fee',
            name: 'paytxfee',
        },
        {
            fieldLabel: 'Minimal input value',
            name: 'mininput',
        }
    ],
    buttons: [
        {
            xtype: 'button',
            text: 'Refresh',
            handler: loadInfo
        },
    ]
});
