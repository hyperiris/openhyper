# Introduction #

Lachesis Shield is a "run as user" windows shell plugin.

# Details #

English:
Lachesis Shield, simple called lshield, is a Windows Shell extension plugin. It's a COM object which implements the IDeskBand interface (IDeskBand2 under Windows 7).

When lshield was loaded by shell, it hooks the CreateProcessW in explorer.exe. When user runs an Application, the lshield modifies the TOKEN by using Win32 Safer API, and use the modified TOKEN to launch the application. So this give us a chance to control the privilege of the new process.

When in exposure mode, TOKEN was NOT modified, so will keeps the same privilege as explorer.exe

When in protect mode, TOKEN will be less privilege, in this mode we can run web browser more safely.

for more detail, see:
http://msdn.microsoft.com/en-us/library/ms972827.aspx

Chinese:
Lachesis Shield, 简称lshield, 是一个Windows Shell扩展. 它是一个COM组件实现了IDeskBand接口(Windows7 下面是IDeskBand2接口).

当系统登录后, 也就是Shell启动, Shell会自动加载lshield, lshield会自动hook explorer.exe的CreateProcessW. 当用户要运行一个应用程序的时候, lshield会使用Win32 Safer API修改令牌, 然后用修改过的令牌来启动应用程序. 这就给了我们一个控制新进程特权的机会.

当处于暴露模式的时候, 令牌保持原样, 新启动的程序和explorer.exe特权一样.

当处于保护模式的时候, 令牌被修改为低特权, 在这个模式下我们可以安全的浏览网页等等.

有关这个技术的详细解释, 可以参考msdn文章:
http://msdn.microsoft.com/zh-cn/library/ms972827.aspx