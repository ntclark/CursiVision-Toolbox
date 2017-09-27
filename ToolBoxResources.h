
#pragma once

#define DISPOSITION_TITLE "Save Locations"

#define IDD_ICON                   1
#define IDD_ICON_PROPERTIES        2
#define IDD_ICON_SAVE_AS           3
#define IDD_ICON_LOGO              4
#define IDD_ICON_CANCEL            5
#define IDD_ICON_DOODLE            6
#define IDD_ICON_START             7
#define IDD_ICON_CREATE            8
#define IDD_ICON_HELP              9
#define IDD_ICON_ABOUT             10
#define IDD_ICON_OPEN              11
#define IDD_ICON_WEB               12
#define IDD_ICON_WEB_HOMEPAGE      13
#define IDD_ICON_WEB_TRAINING      14
#define IDD_ICON_DOODLE_II         15
#define IDD_ICON_CANCEL_DISABLED   16
#define IDD_ICON_PROPERTIES_DISABLED   17
#define IDD_ICON_FORGET            18
#define IDD_ICON_FORGET_DISABLED   19
#define IDD_ICON_UNDO              20
#define IDD_ICON_UNDO_DISABLED     21
#define IDD_ICON_SEAL              22
#define IDD_ICON_SEAL_DISABLED     23
#define IDD_ICON_REPEAT            24
#define IDD_ICON_REPEAT_DISABLED   25
#define IDD_ICON_SAVE_AS_DISABLED  26
#define IDD_ICON_DOODLE_DISABLED   27
#define IDD_ICON_QUICK_DRAW        29
#define IDD_ICON_QUICK_DRAW_WRITE  30
#define IDD_ICON_QUICK_DRAW_WRITE_DISABLED   31

#define ID_ACCELERATORS            30

#define IDBITMAP_BACKGROUND         1

#define PROPERTY_PAGE_WIDTH         360
#define PROPERTY_PAGE_HEIGHT        380

#define IDD_GLOBAL_REPOSITORY       100
#define IDD_DISPOSITION_PROPERTIES  200
#define IDD_DISPOSITION_EMAIL       300
#define IDD_PRINTER_DEVICE_PAGE     400
#define IDD_BACKENDS                900
#define IDD_OPTIONS                 950
#define IDD_DOODLE_OPTIONS          960
#define IDD_DISPOSITION_MULTI_SIGN_PROPERTIES   1100
#define IDD_DISPOSITION_MORE        1200
#define IDD_DISPLAY_WAITING         1300
#if TOPAZ_SUPPORT
#define IDD_VALIDATE_SIGNATURE      1400
#endif

#define IDD_CURSIVISION_CONTROL_OPTIONS   1500
#define IDD_DOCUMENT_TEMPLATE             1600
#define IDD_CURSIVISION_RECOGNITION       1700
#define IDD_SIGNING_LOCATIONS             1800
#define IDD_SIGNING_LOCATIONS_ORDER       1850
#define IDD_DATA_FIELDS                   1900
#define IDD_DATA_FIELDS_LABEL             1950

#define IDD_ABOUT                  1000

#define IDC_EXIT                 200
#define IDC_GETFILE              300
#define IDC_SAVE_AS              301
#define IDC_REOPEN_SOURCE        302
#define IDC_FILENAME             400
#define IDC_CONTROL_HOST         500
#define IDC_UPDATE_PDF           600
#define IDC_SELECT_REGION        700
#define IDC_SELECT_TARGET        701
#define IDC_SELECT_PIECES        702
#define IDC_SELECT_PIECE_TARGETS 703
#define IDC_CLOSE_FILE           704
#define IDC_PRINT                705
#define IDC_SELECT_FONT          706

#define IDC_PROCESS_NAME         800
#define IDC_NEXT                 801
#define IDC_PROCESS_FINISHED     802
#define IDC_PREVIOUS             803
#define IDC_OK                   804
#define IDC_CANCEL               805
#define IDC_DELETE               806
#define IDC_PLAY                 807
#define IDC_PROCESS_CANCELED     808
#define IDC_USE_TEXT             809
#define IDC_USE_IMAGE            810
#define IDC_USE_BOTH             811
#define IDC_FORCE_FIT            812
#define IDC_BRANCH_COUNT         813
#define IDC_CHOICE_1             814
#define IDC_CHOICE_2             815
#define IDC_TARGET_1             816
#define IDC_TARGET_2             817

#define IDR_MAIN_MENU            100
#define IDR_PROCESS_MENU         101
#define ID_PROCESS_OPEN          110
#define ID_PROCESS_NEW           120
#define ID_PROCESS_PLAY          130
#ifdef DO_PROCESSES
#define ID_PROCESS_PLAY_CANCEL   140
#endif
#define ID_HELP_INSTRUCTIONS     150
#define ID_FILE_EXIT             160
#define ID_SIGNATURE_DEVICE      200

#define ID_CURSIVISION_ONLINE                      1303
#define ID_CURSIVISION_ONLINE_HOME                 1304
#define ID_CURSIVISION_ONLINE_TRAINING_VIDEOS      1305
#define ID_ABOUT                 1306
#define ID_QUICK_DRAW            1307
#define ID_QUICK_DRAW_WRITE      1308

#define ID_DOODLE                180
#define ID_DOODLE_CANCEL         1801
#define ID_DOODLE_OPTIONS        181
#define ID_DOODLE_FORGET         182
#define ID_DOODLE_UNDO           183
#define ID_SEAL                  184
#define ID_REPEAT                185
#define ID_SETUP                 190
#define ID_DOODLE_START          191
#define ID_TOPAZ_VALIDATE_SIGNATURE 192
#define ID_RESET_DOODLE_OPTIONS  193
#define ID_MANAGE_PROFILES       194

#define ID_BACKEND_EXECUTE_1     401
#define ID_BACKEND_EXECUTE_MAX   464

#define ID_BACKEND_PROPERTIES_1     501
#define ID_BACKEND_PROPERTIES_MAX   564

#define IDDI_PDF_PANE            300

#define IDDI_GLOBAL_REPOSITORY      101
#define IDDI_GET_GLOBAL_REPOSITORY  102

#define IDDI_DISPOSITION_SAVE                            201
#define IDDI_DISPOSITION_SHOW_STARTUP_PROPERTIES_LABEL   202
#define IDDI_DISPOSITION_SHOW_STARTUP_PROPERTIES         203
#define IDDI_DISPOSITION_APPEND                          205
#define IDDI_DISPOSITION_APPEND_DATE                     206
#define IDDI_DISPOSITION_APPEND_TIME                     207
#define IDDI_DISPOSITION_SUFFIX                          209
#define IDDI_DISPOSITION_SUFFIX_LABEL                    210
#define IDDI_DISPOSITION_REPLACE                         211
#define IDDI_DISPOSITION_SEQUENCE                        212
#define IDDI_DISPOSITION_SAVE_IN_LABEL                   213
#define IDDI_DISPOSITION_SAVE_MY_DOCUMENTS               214
#define IDDI_DISPOSITION_SAVE_BY_ORIGINAL                215
#define IDDI_DISPOSITION_SAVE_IN                         216
#define IDDI_DISPOSITION_SAVE_LOCATION                   217
#define IDDI_DISPOSITION_CHOOSE_SAVE_LOCATION            218
#define IDDI_DISPOSITION_PRINT                           219
#define IDDI_DISPOSITION_PRINTER                         220
#define IDDI_DISPOSITION_SHOW_PROPERTIES                 228
#define IDDI_DISPOSITION_ACCEPT                          230
#define IDDI_DISPOSITION_RETAIN_SIGNED                   231
#define IDDI_DISPOSITION_REOPEN_ORIGINAL                 232
#define IDDI_DISPOSITION_CLOSE_DOCUMENT                  2325
#define IDDI_DISPOSITION_EXIT                            233
#define IDDI_DISPOSITION_OPEN_LAST_DOCUMENT              234
#define IDDI_DISPOSITION_WHILE_SIGNING_PROPERTIES_LABEL  235
#define IDDI_DISPOSITION_WHILE_SIGNING_SHOW_PAD          236
#define IDDI_DISPOSITION_EXIT_POST_LABEL                 237
#define IDDI_DISPOSITION_CONTINUOUS_DOODLE_LABEL         2385
#define IDDI_DISPOSITION_CONTINUOUS_DOODLE_LEARN         238
#define IDDI_DISPOSITION_CONTINUOUS_DOODLE_OFF           239
#define IDDI_DISPOSITION_CONTINUOUS_DOODLE_ON            240
#define IDDI_DISPOSITION_REMEMBER                        241
#define IDDI_HEADER_TEXT                                 242
#define IDDI_DO_REMEMBER_LABEL                           243

#define IDDI_DISPOSITION_MORE                            251
#define IDDI_DISPOSITION_SAVE_MONTHYEAR                  252
#define IDDI_DISPOSITION_SAVE_DAYMONTH                   253
#define IDDI_DISPOSITION_SAVE_VALIDATION                 254

#define IDDI_PRINTING_DEVICE_PROFILES              401
#define IDDI_PRINTING_DEVICE_SKETCH_VIEW           402
#define IDDI_PRINTING_DEVICE_BY_NAME_INSTRUCTIONS  403
#define IDDI_PRINTING_DEVICE_PROFILE_EDIT          404
#define IDDI_PRINTING_DEVICE_DISPOSITION           405
#define IDDI_PRINTING_DEVICE_PROFILE_DELETE        406
#define IDDI_PRINTING_DEVICE_PROFILE_USE_FILENAME  407
#define IDDI_PRINTING_DEVICE_PROFILE_NAME          408
#define IDDI_PRINTING_DEVICE_ASSOCIATE_PRC         409
#define IDDI_PRINTING_DEVICE_ASSOCIATION           410
#define IDDI_PRINTING_DEVICE_ASSOCIATION_FIND      411
#define IDDI_PRINTING_DEVICE_DOODLE_INSTRUCTIONS_1 412
#define IDDI_PRINTING_DEVICE_DOODLE_INSTRUCTIONS_2 413
#define IDDI_PRINTING_DEVICE_PROFILE_SET_GLOBAL    414
#define IDDI_PRINTING_DEVICE_JUST_SIGN_LABEL       418
#define IDDI_PRINTING_DEVICE_JUST_SIGN             419
#define IDDI_PRINTING_DEVICE_MANUAL_SELECTION      420
#define IDDI_PRINTING_DEVICE_MANUAL_CREATE_LABEL   421
#define IDDI_PRINTING_DEVICE_MANUAL_CREATE         422
#define IDDI_PRINTING_ALLOW_NON_ADMIN_ACCESS       423

#define IDDI_PRINTING_DEVICE_PROFILE_OK         620
#define IDDI_PRINTING_DEVICE_PROFILE_CANCEL     621

#define IDDI_PRINTING_VIEW_PROFILE_NAME         701
#define IDDI_PRINTING_VIEW_INSTRUCTIONS         702
#define IDDI_PRINTING_VIEW_VIEW                 703
#define IDDI_PRINTING_VIEW_REGION_DELETE        704
#define IDDI_PRINTING_VIEW_OK                   705
#define IDDI_PRINTING_VIEW_CANCEL               706
#define IDDI_PRINTING_VIEW_POINT_SELECT         707
#define IDDI_PRINTING_VIEW_DRAG_SELECT          708
#define IDDI_PRINTING_VIEW_REGIONS_RESET        709

#define IDDI_BACKENDS_LIST_LABEL                901
#define IDDI_BACKENDS_LIST                      902

#define IDDI_BACKENDS_USE_BACKEND               910
#define IDDI_BACKENDS_USE_BACKEND_MAX           942
#define IDDI_BACKENDS_PROPERTIES                943
#define IDDI_BACKENDS_PROPERTIES_MAX            975
#define IDDI_BACKENDS_ORDER                     976
#define IDDI_BACKENDS_ORDER_MAX                1008

#define IDDI_ASSOCIATION_MESSAGE               1009
#define IDDI_BACKENDS_TOP_LIST_LABEL           1010
#define IDDI_BACKENDS_TOP_LIST                 1011
#define IDDI_BACKENDS_BOTTOM_LIST_LABEL        1012
#define IDDI_BACKENDS_BOTTOM_LIST              1013
#define IDDI_BACKENDS_MOVE_TO_TOP_LIST         1014
#define IDDI_BACKENDS_REMOVE_FROM_TOP_LIST     1015

#define IDDI_BACKENDS_EMAIL_FROM                301
#define IDDI_BACKENDS_EMAIL_TO                  302
#define IDDI_BACKENDS_EMAIL_CC                  303
#define IDDI_BACKENDS_EMAIL_BCC                 304
#define IDDI_BACKENDS_EMAIL_SUBJECT             305
#define IDDI_BACKENDS_EMAIL_BODY                306
#define IDDI_BACKENDS_EMAIL_BODY_EDIT           307   
#define IDDI_BACKENDS_EMAIL_SERVER              308
#define IDDI_BACKENDS_EMAIL_PORT                309
#define IDDI_BACKENDS_EMAIL_USERNAME            310
#define IDDI_BACKENDS_EMAIL_PASSWORD            311
#define IDDI_BACKENDS_EMAIL_SHOWDIALOG          312
#define IDDI_BACKENDS_EMAIL_OK                  313
#define IDDI_BACKENDS_EMAIL_CANCEL              314

#define IDDI_TOOLBOX_NEED_ADMIN_PRIVILEGES      1024

#define IDDI_OPTIONS_TEXT                       951
#define IDDI_OPTIONS_OPTION_1                   952
#define IDDI_OPTIONS_OPTION_2                   953
#define IDDI_OPTIONS_OPTION_3                   954

#define IDDI_DOODLE_OPTIONS_LABEL1              961
#define IDDI_DOODLE_OPTIONS_LABEL2              962
#define IDDI_DOODLE_OPTIONS_LABEL3              963
#define IDDI_DOODLE_OPTIONS_LABEL4              964
#define IDDI_DOODLE_OPTIONS_LABEL5              965
#define IDDI_DOODLE_OPTIONS_REMEMBER            966
#define IDDI_DOODLE_OPTIONS_FORGET              967
#define IDDI_DOODLE_OPTIONS_OWN_DISPOSITION     968
#define IDDI_DOODLE_OPTIONS_DISPOSITION         969
#define IDDI_DOODLE_OPTIONS_RESETBUTTON         970
#define IDDI_DOODLE_OPTIONS_LEFTBUTTON          971
#define IDDI_DOODLE_OPTIONS_RIGHTBUTTON         972
#define IDDI_DOODLE_OPTIONS_ERASEBUTTON         973
#define IDDI_EVALUATION_MESSAGE                 974

#define IDDI_ABOUT_OK                          1001
#define IDDI_ABOUT_TEXT_0                      1002
#define IDDI_ABOUT_TEXT_1                      1003
#define IDDI_ABOUT_TEXT_2                      1004
#define IDDI_ABOUT_TEXT_3                      1005
#define IDDI_ABOUT_ICON                        1006


#define IDDI_DISPLAY_WAITING_LABEL1            1301
#define IDDI_DISPLAY_WAITING_LABEL2            1302
#define IDDI_DISPLAY_WAITING_SHOW              1303

#define IDDI_VALIDATION_SIGNED_FILE            1401
#define IDDI_VALIDATION_SIGNED_FILE_GET        1402
#define IDDI_VALIDATION_SIG_FILE               1403
#define IDDI_VALIDATION_SIG_FILE_GET           1404
#define IDDI_VALIDATION_SIGNATURE_PAD_HOST     1405
#define IDDI_VALIDATION_VALIDATE               1406

#define IDDI_VALIDATION_OK                     1410

#define IDDI_SIGNING_LOCATIONS_LABEL1          1701
#define IDDI_SIGNING_LOCATIONS_LABEL2          1702
#define IDDI_SIGNING_LOCATIONS_LABEL3          1703
#define IDDI_SIGNING_LOCATIONS_VIEW            1710
#define IDDI_SIGNING_LOCATIONS_SCROLL          1711
#define IDDI_SIGNING_LOCATIONS_NEW             1720
#define IDDI_SIGNING_LOCATIONS_DELETE          1721
#define IDDI_SIGNING_LOCATIONS_CUT             1722
#define IDDI_SIGNING_LOCATIONS_COPY            1723
#define IDDI_SIGNING_LOCATIONS_PASTE           1724
#define IDDI_SIGNING_LOCATIONS_SET_ORDER       1725

#define IDDI_SIGNING_LOCATIONS_ORDER           1801
#define IDDI_SIGNING_LOCATIONS_ORDER_SPIN      1802
#define IDDI_SIGNING_LOCATIONS_ORDER_LABEL     1803
#define IDDI_SIGNING_LOCATIONS_ORDER_OK        1804
#define IDDI_SIGNING_LOCATIONS_ORDER_CANCEL    1805

#define IDDI_CV_CONTROL_TEMPLATE          100
#define IDDI_CV_CONTROL_TEMPLATE_GET      101
#define IDDI_CV_DOCUMENT                  102
#define IDDI_CV_DOCUMENT_SCROLL           103
#define IDDI_CV_RECOGNITION_INSTRUCTIONS  104
#define IDDI_CV_LOCATIONS_RESET           105
#define IDDI_CV_LIMIT_REACHED             106
#define IDDI_CV_LOCATIONS_INSTRUCTIONS    107
#define IDDI_CV_LOCATIONS_ADDITIONAL_INFO 108
#define IDDI_CV_MORE_INFORMATION          109

#define IDDI_DATA_FIELDS_RESET         1901
#define IDDI_DATA_FIELDS_INSTRUCTIONS  1902
#define IDDI_DATA_FIELDS_DELETE        1903
#define IDDI_DATA_FIELDS_LABEL         1904
#define IDDI_FIELDS_LABEL_LABEL        1951
#define IDDI_FIELDS_VALUE_REQUIRED     1952
#define IDDI_FIELDS_LABEL_OK           1953
#define IDDI_FIELDS_LABEL_CANCEL       1954

#define IDDI_PROFILE_INSTRUCTIONS      2001

#define IDS_MAIN_PAGE_0          1
#define IDS_MAIN_PAGE            2
#define IDS_MAIN_PAGE_MORE       3
#define IDS_IMAGE_PAGE           4
#define IDS_IMAGE_PAGE_MORE      5
#define IDS_PROPERTIES           6
#define IDS_PROPERTYPAGES        7
#define IDS_PDF_ENABLER          8
#define IDS_PRINTING_SUPPORT     10
#define IDS_SIGPLUS              11
#define IDS_ABOUT_0              12
#define IDS_ABOUT_1              13
#define IDS_ABOUT_2              14
#define IDS_ABOUT_3              15
#define IDS_FILE_ASSOCIATION_1   16
#define IDS_FILE_ASSOCIATION_2   17
#define IDS_IS_ASSOCIATED        18
#define IDS_NOT_REGISTERED       19
#define IDS_PROCESS_INSTRUCTIONS 20
#define IDS_PROCESS_IMAGE_INSTRUCTIONS 21
#define IDS_PROCESS_TEXT_INSTRUCTIONS  22
#define IDS_PROCESS_BOTH_INSTRUCTIONS  23
#define IDS_PROCESS_NONLCD_INSTRUCTIONS 24
#define IDS_WELCOME              25
#define IDS_BAD_READER           26
#define IDS_CURSIVISION_CONTROL  27
#define IDS_PAD_NOT_CONNECTED    28

#define WM_USER_MIN                 (WM_USER + 101)
#define WM_FINISH_SIGNATURE_PLAY    (WM_USER + 101)
#define WM_FIND_READER              (WM_USER + 102)
#define WM_BRING_TO_TOP             (WM_USER + 105)
#define WM_FIND_READER_STAGE_2      (WM_USER + 106)
#define WM_IMMEDIATE_DOODLE         (WM_USER + 107)
#define WM_OPEN_PDF_FILE            (WM_USER + 108)
#define WM_PDF_FILE_OPENED          (WM_USER + 109)
#define WM_CLEANUP_SIGNATURE_PLAY   (WM_USER + 110)
#define WM_START_PRINT_PROCESSING   (WM_USER + 111)
#define WM_START_ADHOC              (WM_USER + 112)
#define WM_SET_ACTIVE               (WM_USER + 113)
#define WM_POTENTIAL_QUIT           (WM_USER + 114)
#define WM_FINISH_SIGNATURE_PAGE    (WM_USER + 115)
#define WM_FINISH_SIGNATURE_PAGE_2  (WM_USER + 116)
#define WM_PRINT_ONLY               (WM_USER + 117)
#define WM_CONTINUE_PLAY            (WM_USER + 118)
#define WM_PREPARE_SIGNATURE_REPLAY (WM_USER + 119)
#define WM_PLAY_NEXT_PAGE           (WM_USER + 120)
#define WM_CAN_PAD_GET_SIG_DATA     (WM_USER + 121)
#define WM_POST_PAINT               (WM_USER + 122)
#define WM_REGISTER_PAD             (WM_USER + 123)
#define WM_DISPLAY_SIGNATURE_BOX    (WM_USER + 124)
#define WM_APPLY_SIGNATURE          (WM_USER + 125)
#define WM_FIRE_DOCUMENT_OPENED     (WM_USER + 126)
#define WM_FIRE_DOCUMENT_CLOSED     (WM_USER + 127)
#define WM_FORCE_SHUTDOWN           (WM_USER + 128)

#define WM_USER_MAX                 (WM_USER + 128)

#define TIMER_EVENT_MIN_ID             1

#define TIMER_EVENT_FIND_READER        1

#define TIMER_EVENT_BRING_TO_TOP       3
#define TIMER_EVENT_START_DOODLE       4
#define TIMER_EVENT_END_SIGNATURE      5
#define TIMER_EVENT_REDOODLE           7
#define TIMER_EVENT_CONTINUE_PLAY      8
#define TIMER_EVENT_TRACK_PAGE_NUMBER  9
#define TIMER_EVENT_PRINT_ONLY         10
#define TIMER_EVENT_REESTABLISH_PENHOST         12
#define TIMER_EVENT_START_DOODLE_PART2          13
#define TIMER_EVENT_DISPLAY_WAITING             14
#define TIMER_EVENT_REESTABLISH_PENHOST_READERX 15
#define TIMER_EVENT_PDF_FILE_OPENED    17

#define TIMER_EVENT_MAX_ID             17

#define FIND_PAGE_TIMER_DURATION       500
#define ESTABLISH_PENHOST_DURATION     100
#ifdef CURSIVISION_CONTROL_BUILD
#define DOODLE_START_DELAY             500
#else
#define DOODLE_START_DELAY             2500
#endif
#define RE_DOODLE_DELAY                500
#define START_DOODLE_PART_2_DELAY      100
#define PDF_OPENED_DELAY               400

#ifdef CURSIVISION_CONTROL_BUILD
#define FIND_READER_DELAY              500
#define FIND_READER_DELAY_REFIRE       750
//#define FIND_READER_DELAY              500
//#define FIND_READER_DELAY_REFIRE       75
#else
#define FIND_READER_DELAY              2000
#define FIND_READER_DELAY_REFIRE       1000
#endif
