
rem for /R .\ %%f in (*.c,*.cpp) do astyle --style=ansi  --indent=spaces=2 %%f


rem for /R .\ %%f in (*.c,*.cpp) do astyle --style=ansi  --indent=tab=2 %%f --min-conditional-indent=0


for /R .\ %%f in (*.c,*.cpp,*.h) do astyle --style=ansi --indent=tab=4 --indent-cases  --pad-oper --pad-header --align-pointer=name --add-brackets --max-code-length=160 --break-after-logical %%f

for /R .\ %%f in (*.c.orig,*.h.orig,*.cpp.orig) do del /s /f %%f
