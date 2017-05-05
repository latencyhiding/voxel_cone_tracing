let SessionLoad = 1
if &cp | set nocp | endif
let s:so_save = &so | let s:siso_save = &siso | set so=0 siso=0
let v:this_session=expand("<sfile>:p")
silent only
cd ~/projects/voxel_cone_tracing
if expand('%') == '' && !&modified && line('$') <= 1 && getline(1) == ''
  let s:wipebuf = bufnr('%')
endif
set shortmess=aoO
badd +249 src/renderer.cpp
badd +36 shader/passthrough.frag
badd +2 shader/passthrough.vert
badd +91 src/renderer.h
badd +152 src/main.cpp
badd +54 shader/voxelize.geom
badd +1 src/device.cpp
badd +19 shader/voxel_cone_tracing.vert
badd +237 shader/voxel_cone_tracing.frag
badd +95 shader/voxelize.frag
badd +131 lib/gl_utils/src/gl_helpers.c
badd +28 shader/voxelize.vert
badd +8 src/texture_3d.h
badd +32 src/texture_3d.cpp
badd +51 CMakeLists.txt
badd +11 shader/mipmap.comp
badd +89 ~/.vimrc
badd +350 thirdparty/imgui/examples/opengl3_example/imgui_impl_glfw_gl3.cpp
badd +6 .syntastic_cpp_config
argglobal
silent! argdel *
argadd src/renderer.cpp
edit src/main.cpp
set splitbelow splitright
wincmd _ | wincmd |
vsplit
1wincmd h
wincmd w
wincmd _ | wincmd |
split
wincmd _ | wincmd |
split
2wincmd k
wincmd w
wincmd w
set nosplitbelow
set nosplitright
wincmd t
set winheight=1 winwidth=1
exe 'vert 1resize ' . ((&columns * 135 + 135) / 271)
exe '2resize ' . ((&lines * 22 + 35) / 71)
exe 'vert 2resize ' . ((&columns * 135 + 135) / 271)
exe '3resize ' . ((&lines * 23 + 35) / 71)
exe 'vert 3resize ' . ((&columns * 135 + 135) / 271)
exe '4resize ' . ((&lines * 22 + 35) / 71)
exe 'vert 4resize ' . ((&columns * 135 + 135) / 271)
argglobal
setlocal fdm=manual
setlocal fde=0
setlocal fmr={{{,}}}
setlocal fdi=#
setlocal fdl=0
setlocal fml=1
setlocal fdn=20
setlocal fen
silent! normal! zE
let s:l = 85 - ((8 * winheight(0) + 34) / 69)
if s:l < 1 | let s:l = 1 | endif
exe s:l
normal! zt
85
normal! 059|
wincmd w
argglobal
edit src/renderer.h
setlocal fdm=manual
setlocal fde=0
setlocal fmr={{{,}}}
setlocal fdi=#
setlocal fdl=0
setlocal fml=1
setlocal fdn=20
setlocal fen
silent! normal! zE
let s:l = 187 - ((10 * winheight(0) + 11) / 22)
if s:l < 1 | let s:l = 1 | endif
exe s:l
normal! zt
187
normal! 0
wincmd w
argglobal
edit src/renderer.cpp
setlocal fdm=manual
setlocal fde=0
setlocal fmr={{{,}}}
setlocal fdi=#
setlocal fdl=0
setlocal fml=1
setlocal fdn=20
setlocal fen
silent! normal! zE
let s:l = 374 - ((11 * winheight(0) + 11) / 23)
if s:l < 1 | let s:l = 1 | endif
exe s:l
normal! zt
374
normal! 03|
wincmd w
argglobal
edit shader/voxel_cone_tracing.frag
setlocal fdm=manual
setlocal fde=0
setlocal fmr={{{,}}}
setlocal fdi=#
setlocal fdl=0
setlocal fml=1
setlocal fdn=20
setlocal fen
silent! normal! zE
let s:l = 244 - ((2 * winheight(0) + 11) / 22)
if s:l < 1 | let s:l = 1 | endif
exe s:l
normal! zt
244
normal! 014|
wincmd w
exe 'vert 1resize ' . ((&columns * 135 + 135) / 271)
exe '2resize ' . ((&lines * 22 + 35) / 71)
exe 'vert 2resize ' . ((&columns * 135 + 135) / 271)
exe '3resize ' . ((&lines * 23 + 35) / 71)
exe 'vert 3resize ' . ((&columns * 135 + 135) / 271)
exe '4resize ' . ((&lines * 22 + 35) / 71)
exe 'vert 4resize ' . ((&columns * 135 + 135) / 271)
tabnext 1
if exists('s:wipebuf')
  silent exe 'bwipe ' . s:wipebuf
endif
unlet! s:wipebuf
set winheight=1 winwidth=20 shortmess=filnxtToO
let s:sx = expand("<sfile>:p:r")."x.vim"
if file_readable(s:sx)
  exe "source " . fnameescape(s:sx)
endif
let &so = s:so_save | let &siso = s:siso_save
let g:this_session = v:this_session
let g:this_obsession = v:this_session
let g:this_obsession_status = 2
doautoall SessionLoadPost
unlet SessionLoad
" vim: set ft=vim :
