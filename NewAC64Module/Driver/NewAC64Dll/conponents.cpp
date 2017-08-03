
#include "stdafx.h"

#include "utils.h"

#include "drv_peb64.h"
#include "api_peb64.h"
#include "conponents.h"

#include <stdio.h>
#include <stdlib.h>
#include <Strsafe.h>
#include <String.h>

#if _DEBUG
#include <conio.h>
#endif


void conponents::ComboBoxScanSlot( CComboBox * pCComboSlot )
{
	int oriSel = pCComboSlot->GetCurSel();

	pCComboSlot->ResetContent();
	
	int firstPeb64SlotInd = -1;
	int totPeb64Slots = 0;
	for ( int slotNum=1; slotNum<=getSupportSlotAmount(); slotNum++ )
	{
        TCHAR const * pMainBoardName;
        pMainBoardName = apiDscvMainBrdName( slotNum );
        
        TCHAR pCotentBuf[200];
        StringCbPrintf( pCotentBuf, sizeofarray(pCotentBuf), _T("Slot %2d: %s "), slotNum, pMainBoardName );
        
		pCComboSlot->AddString( pCotentBuf );
		int index = pCComboSlot->GetCount()-1;

		int itemInd = slotNum-1;
		if ( wcscmp( pMainBoardName, _T("PEB64")) == 0 )
		{
			pCComboSlot->SetItemData( itemInd, RLY_CB_DATA_SUPPORTED );
			if ( totPeb64Slots == 0 ) firstPeb64SlotInd = itemInd;
			totPeb64Slots++;
		}
		else
		{
			pCComboSlot->SetItemData( itemInd, RLY_CB_DATA_UNSUPPORT );
		}
	}

	if ( oriSel != -1 )
        pCComboSlot->SetCurSel( oriSel );
    else
        pCComboSlot->SetCurSel( 0 );
	
	// Select the only PEB64 slot
	if ( totPeb64Slots == 1 && pCComboSlot->GetCurSel() != firstPeb64SlotInd )
	{
		pCComboSlot->SetCurSel( firstPeb64SlotInd );
	}
}

void conponents::AppendTextToEditCtrl( CEdit * pedit, LPCTSTR strLine)
{
	// get the initial text length
	int nLength = pedit->GetWindowTextLength();
	// put the selection at the end of text
	pedit->SetSel(nLength, nLength);
	// replace the selection
	pedit->ReplaceSel(strLine);
}