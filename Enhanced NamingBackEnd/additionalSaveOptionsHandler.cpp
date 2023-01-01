// Copyright 2017 InnoVisioNate Inc. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "EnhancedNamingBackEnd.h"

#define ADDITIONAL_INITIALIZATION       \
    SetDlgItemText(hwnd,IDDI_HEADER_TEXT,"Please specify the storage options for the resulting document:");

#define OBJECT_WITH_PROPERTIES NamingBackEnd

#include "additionalSaveOptionsBody.cpp"
