#pragma once

#define RLY_CB_DATA_UNSUPPORT   0
#define RLY_CB_DATA_SUPPORTED   1

#define RLY_LB_DATA_ON          1
#define RLY_LB_DATA_OFF         0
#define RLY_LB_DATA_ERROR       2
#define RLY_LB_DATA_UNSUPPORT   3
namespace conponents
{
	void ComboBoxScanSlot( CComboBox * pCComboSlot );
	void AppendTextToEditCtrl( CEdit * pedit, LPCTSTR strLine);
};
