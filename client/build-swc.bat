@echo off

if not exist build mkdir build
pushd build

REM call tsc ../src/game.ts --outfile game.js

call swc compile ../src/game.ts --out-file game.js

if "%1" == "rel" (
	rem optimize js
	rem ( there are other js files but they're already tiny and/or minified )
	call terser game.js -o game.js
	REM it's not game.min.js because then we'd need two HTML files or conditionally generate them
)


copy ..\src\index.html index.html

REM tsc / swc handles this one
rem copy ..\src\game.js game.js

copy ..\src\samjs.min.js samjs.min.js
copy ..\src\riffwave.js riffwave.js
copy ..\src\sfxr.js sfxr.js

if "%1" == "rel" (
	rem gzip compress the files for efficient transfer to browser
	7z a -tgzip index.html.gz index.html
	7z a -tgzip game.js.gz game.js
	7z a -tgzip samjs.min.js.gz samjs.min.js
	7z a -tgzip riffwave.js.gz riffwave.js
	7z a -tgzip sfxr.js.gz sfxr.js
)

popd