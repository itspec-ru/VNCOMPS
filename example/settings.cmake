
#==============================================================================#
# identification you AddIn
SET(MYCOMPANY "MyCompany" CACHE STRING "You company name")
SET(ADN_NAME "AddInExample" CACHE STRING "You addin name")
#==============================================================================#
set(Addn_VERSION_MAJOR 1 CACHE STRING "Version major")
set(Addn_VERSION_MINOR 4 CACHE STRING "Version minor")
set(Addn_VERSION_BUILD 4 CACHE STRING "Version build")
SET(Addn_VERSION_EDITION 06 CACHE STRING "Edition")
#==============================================================================#
#seting projects names
SET(Addn_Native "AddInNative" CACHE STRING "Name for NativeAPI project")
SET(Addn_Chrome "AddInChr" CACHE STRING "Name for Chrome adapter project")
SET(Addn_IE "AddInIE" CACHE STRING "Name for IE adapter project")
SET(Addn_ChromeSetup "AddInChromeSetup" CACHE STRING "Name for Chrome Setup project" )
SET(Addn_IESetup "AddInIESetup" CACHE STRING "Name for IE Setup project")
SET(Addn_Safari "AddInSaf" CACHE STRING "Name for Safari adapter project")
SET(Addn_COM "AddInCOM" CACHE STRING "Name for COM project")
SET(Addn_Mainfest "com.mycompany.enterprise.addin.${Addn_VERSION_MAJOR}.${Addn_VERSION_MINOR}.${Addn_VERSION_BUILD}.${Addn_VERSION_EDITION}" CACHE STRING "Name of manifest for Chrome and Firefox")
SET(Addn_MimeType "application/component-${Addn_VERSION_MAJOR}.${Addn_VERSION_MINOR}.${Addn_VERSION_BUILD}.${Addn_VERSION_EDITION}" CACHE STRING "MIMEType for NPAPI project")
#==============================================================================#
