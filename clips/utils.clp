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

; datetime.clock compare
(deffunction timeout(?t1 ?t2 ?timeout)
    return (> (- ?t1 ?timeout) ?t2)
)

; Condition => Action: trigger device control
(deffunction act-control (?id ?slot ?value)
    (if (instance-existp ?id)
     then
        (if (numberp ?value)
         then
            (bind ?val (str-cat # ?value))
            (bind ?eqsym =)
         else
            (bind ?val ?value)
            (bind ?eqsym eq)
        )
        (printout debug "device control(" ?id ", " ?slot ", " ?val ")" crlf)
        (bind ?result (ins-push ?id ?slot ?val))
        (bind ?clsname (symbol-to-instance-name ?id))
        (return (str-cat "?"
                 ?id " <- (object (is-a " ?clsname ") (" 
                 ?slot " ?" ?slot " &:(" ?eqsym " ?" ?slot " " ?value")))"))
     else
        (printout warn "NOT FOUND: " ?id " instance" crlf)
    )
    (return FALSE)
)

; Condition => Action: trigger content notify
(deffunction act-notify (?id ?title ?content)
    (if (and (lexemep ?title) (lexemep ?content))
     then
        (printout debug "act_notify(" ?id ", " ?title ", " ?content ")" crlf)
        (txt-push ?id ?title ?content)
     else
        (printout warn "Parameters is invalid: (" ?id ", " ?title ", " ?content ")" crlf)
    )
)

; Condition => Action: trigger scene list
(deffunction act-scene (?type $?ruleid-list)
    (if (eq ?type list)
     then
        (foreach ?ruleid (create$ $?ruleid-list)
            ; Check the ruleid exist
            (if (defrule-module ?ruleid)
              then
                (printout debug "(act-scene " ?ruleid ")" crlf)
                (assert (scene ?ruleid))
              else
                (printout warn "NOT FOUND: scene:" ?ruleid crlf)
            )
        )
     else
        (printout debug "(act-scene " ?ruleid-list ")" crlf)
        (assert (scene ?ruleid-list))
    )
)

(deffunction handle-control-result (?timeout ?ruleid $?cond-list)
    (foreach ?cond (create$ $?cond-list)
        (if (eq ?cond FALSE)
         then 
            (return FALSE)
        )
        (printout debug "---> " ?cond crlf)
    )

    ; => (assert 
    ; (if (or
         ; (eq ?act1 FALSE)
         ; (eq ?act2 FALSE)
         ; (eq ?act3 FALSE)
         ; (eq ?act4 FALSE)
        ; )
)
