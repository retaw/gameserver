"show line num
set number

"auto file type
filetype on
filetype indent on

"1 tab == 4 space
set tabstop=4

"auto indent and indent width == 4 space
set smartindent
set shiftwidth=4

"auto replace tab by 4 spaces
set smarttab
set expandtab

"high light search result
set hlsearch

"case sensitive is smart
set ignorecase smartcase 

set fileencodings=utf-8,utf-16,gb18030,gb2312,gbk,utf-32
"set termencoding=utf-8
"set encoding=prc

"auto indent when typing c/c++ code
set cindent

"set style of c/c++ code indent, not but almost perfect
set cinoptions=0-s,g0,i0.5s,+0,:0,l0.5s,(0,U1,w1,t0

"increase search
set incsearch

"paste mode
set pastetoggle=<F9>

"add file header
map <F10> :call TitleDet()<cr>'s
function AddTitle()
    call append(00,"/*")
    call append(01," * Author: LiZhaojia ")
    call append(02," *")
    call append(03," * Last modified: ".strftime("%Y-%m-%d %H:%M %z"))
    call append(04," *")
    call append(05," * Description: ")
    call append(06," */")
    echohl WarningMsg | echo "Successful in adding the copyright." | echohl None
endf
"update file header
function UpdateTitle()
    normal m'
    "execute '/# *Last modified:/s@:.*$@\=strftime(":\t%Y-%m-%d %H:%M")@'
    execute '/\s\*\sLast modified:/s@:.*$@\=strftime(": %Y-%m-%d %H:%M %z")@'
    normal ''
    echohl WarningMsg | echo "Successful in updating the copy right." | echohl None
endfunction
"add or update fileheader
function TitleDet()
    let n=1
    while n < 6
        let line = getline(n)
        if line =~ '^\s\*\sLast\smodified:'
            call UpdateTitle()
            return
        endif
        let n = n + 1
    endwhile
    call AddTitle()
endfunction

