
#include "TaxWise Naming.h"

#define ADDITIONAL_INITIALIZATION       \
    SetDlgItemText(hwnd,IDDI_HEADER_TEXT,"Please specify the storage options for the resulting document:");

#define OBJECT_WITH_PROPERTIES NamingBackEnd

#include "additionalSaveOptionsBody.cpp"
