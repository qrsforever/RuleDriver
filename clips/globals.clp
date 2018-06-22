;=================================================================
; date: 2018-06-14 19:11:27
; title: All Global Construct
;=================================================================

;-----------------------------------------------------------------
;	Global varible
;       1. Use ?*var* to access the valuea of global varible
;       2. Use bind function to set the value of global varible.
;-----------------------------------------------------------------

; Salience Level Define
(defglobal
    ?*SALIENCE-HIGHEST*  =  10000
    ?*SALIENCE-HIGHER*   =   5000
    ?*SALIENCE-HIGH*     =   1000
    ?*SALIENCE-NORMAL*   =      0
    ?*SALIENCE-LOWER*    =  -1000
    ?*SALIENCE-LOWEST*   = -10000
)

; Log Level Define: must be consisence with defined in Log.h
(defglobal
    ?*LOG-LEVEL-ERROR*      = 1
    ?*LOG-LEVEL-WARNING*    = 2
    ?*LOG-LEVEL-DEBUG*      = 3
    ?*LOG-LEVEL-INFO*       = 4
    ?*LOG-LEVEL-TRACE*      = 5
)

;-----------------------------------------------------------------
;	Global Template
;-----------------------------------------------------------------

;-----------------------------------------------------------------
;	Global Class
;-----------------------------------------------------------------

; Base Device Abstract
(defclass DEVICE (is-a USER) (role abstract)
    (slot ID     (visibility public) (type SYMBOL))
    (slot Class  (visibility public) (type SYMBOL) (access initialize-only))
    (slot Parent (visibility public) (default-dynamic nil))
    (slot UUID   (visibility public) (type STRING))
    (slot insCnt (type INTEGER) (storage shared) (default 0))
)

(defmessage-handler DEVICE init after ()
    (bind ?self:ID (instance-name-to-symbol (instance-name ?self)))
    (bind ?self:Class (class ?self))
    (bind ?self:insCnt (+ ?self:insCnt 1))
)

;-----------------------------------------------------------------
;	Global Function
;-----------------------------------------------------------------

; Condition => Action: trigger device control
(deffunction act-control (?id ?slot ?value)
    (if (instance-existp ?id)
     then
        (if (numberp ?value)
         then
            (bind ?value (str-cat # ?value))
        )
        (printout debug "device control(" ?id ", " ?slot ", " ?value ")" crlf)
        (ins-push ?id ?slot ?value)
     else
        (printout warn "NOT FOUND: " ?id " instance" crlf)
    )
)

; Condition => Action: trigger message notify
(deffunction act-notify (?title ?message)
    (if (and (lexemep ?title) (lexemep ?message))
     then
        (printout debug "act_notify(" ?title ", " ?message ")" crlf)
        (msg-push ?title ?message)
     else
        (printout warn "Parameters is invalid: (" ?title ", " ?message ")" crlf)
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
                (assert (scene ?ruleid))
              else
                (printout warn "NOT FOUND: scene:" ?ruleid crlf)
            )
        )
    )
)

;-----------------------------------------------------------------
;	Global Rule
;-----------------------------------------------------------------

; show facts, ruels, instances and so on debug info
(defrule show-elem
    ?f <- (show ?elem)
  =>
    (retract ?f)
    (switch ?elem
        (case instances then (instances))
        (case facts then (facts))
        (case rules then (rules))
        (case agenda then (agenda))
        (default (printout warn "Unkown elem: " ?elem crlf))
    )
)

; (time (now)) from program
(defrule retract-time
    (declare (salience ?*SALIENCE-LOWEST*))
    ?f <- (time $?)
  =>
    (if (>= ?*LOG-LEVEL* ?*LOG-LEVEL-INFO*)
      then
        (facts)
    )
    (retract ?f)
)
