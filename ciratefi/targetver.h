
#pragma once

// 下列巨集會定義最低平台需求。最低平台需求是指各種版本的
// Windows、Internet Explorer 中，具備執行應用程式所需功能的
// 最早版本。巨集的作用，是在指定或更新版本的平台上啟用
// 所有可用的功能。

// 如果您有必須優先選取的平台，請修改下列定義。
// 參考 MSDN 取得不同平台對應值的最新資訊。
#ifndef WINVER                          // 指定最低平台需求為 Windows Vista。
#define WINVER 0x0600           // 將它變更為針對 Windows 其他版本的適當值。
#endif

#ifndef _WIN32_WINNT            // 指定最低平台需求為 Windows Vista。
#define _WIN32_WINNT 0x0600     // 將它變更為針對 Windows 其他版本的適當值。
#endif

#ifndef _WIN32_WINDOWS          // 指定最低平台需求為 Windows 98。
#define _WIN32_WINDOWS 0x0410 // 將它變更為針對 Windows Me (含) 以後版本的適當值。
#endif

#ifndef _WIN32_IE                       // 指定最低平台需求為 Internet Explorer 7.0。
#define _WIN32_IE 0x0700        // 將它變更為針對 IE 其他版本的適當值。
#endif

