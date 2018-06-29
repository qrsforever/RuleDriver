;=================================================================
; date: 2018-06-14 18:55:54
; title: utils
;=================================================================

(deffunction load-file (?file)
    (bind ?pos (str-index "." ?file))
    (if (neq ?pos FALSE)
     then
        (bind ?slash (str-index "/" ?file))
        (bind ?path (resolve-file ?file ?slash))
        (if (eq ?path FALSE)
         then
            (return)
        )
        (bind ?suffix (sub-string (+ ?pos 1) (length ?file) ?file))
        (switch ?suffix
            (case "clp"
             then
                (load* ?path)
            )
            (case "bat"
             then
                (batch* ?path)
            )
            (default (printout warn "Cannot load file: " ?file crlf))
        )
     else
        (printout warn "Only handle .clp or .bat files!" crlf)
    )
)

(deffunction load-files ($?file-list)
    (bind ?end (length$ $?file-list))
    (loop-for-count (?n 1 ?end)
     do
        (load-file (nth$ ?n $?file-list))
    )
)
