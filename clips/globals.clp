;=================================================================
; date: 2018-06-14 19:11:27
; title: All Global Construct
;=================================================================

;-----------------------------------------------------------------
;    Global varible
;       1. Use ?*var* to access the value of global varible
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
;   Global Template
;-----------------------------------------------------------------

;-----------------------------------------------------------------
;    Global Class
;-----------------------------------------------------------------

; Rule Context
(defclass RuleContext (is-a USER) (role concrete)
    (slot rule-id (type STRING))
    (slot start-time (type INTEGER) (default-dynamic (nth$ 1 (now))))
    (slot end-time (type INTEGER))
)

(defmessage-handler RuleContext init after (?timeout)
    (bind ?self:end-time (+ ?self:start_time ?timeout))
)

; Base Device Abstract
(defclass DEVICE (is-a USER) (role abstract)
    (slot ID     (visibility public) (type SYMBOL))
    (slot Class  (visibility public) (type SYMBOL) (access initialize-only))
    (slot UUID   (visibility public) (type STRING))
    (slot insCnt (type INTEGER) (storage shared) (default 0))
)

(defmessage-handler DEVICE init after ()
    (bind ?self:ID (instance-name-to-symbol (instance-name ?self)))
    (bind ?self:Class (class ?self))
    (bind ?self:insCnt (+ ?self:insCnt 1))
)

;-----------------------------------------------------------------
;    Global Rule
;-----------------------------------------------------------------

; show facts, ruels, instances and so on debug info
(defrule show-elem
    ?f <- (show ?elem)
  =>
    (retract ?f)
    (switch ?elem
        (case instances then (instances))
        (case facts     then    (facts))
        (case rules     then    (rules))
        (case agenda    then    (agenda))
        (case classes   then    (list-defclasses))
        (case globals   then    (show-defglobals))
        (case memory    then    (printout info "Memory Used:"(/ (mem-used) 131072)" MB" crlf))
        (default (printout warn "Unkown elem: " ?elem crlf))
    )
)

; (datetime (now)) from program
(defrule retract-datetime
    (declare (salience ?*SALIENCE-LOWEST*))
    ?f <- (datetime $?)
  =>
    (if (>= ?*LOG-LEVEL* ?*LOG-LEVEL-TRACE*)
      then
        (facts)
    )
    (retract ?f)
)

; delete one rule in clp script when (assert (delete-rule ruleid-1 ruleid-2))
(defrule delete-rule
    ?f <- (delete-rule $?ruleid-list)
  =>
    (retract ?f)
    (foreach ?ruleid (create$ $?ruleid-list)
        (if (defrule-module ?ruleid)
         then
            (undefrule ?ruleid)
        )
    )
)

; check rule response with timeout
(defrule check-rule-timeout-try
    (datetime ?clock $?other)
    ?obj <- (object (is-a RuleContext) (end-time ?end &:(< ?end ?clock)))
  =>
   ; (bind ?count (send ?obj get-try-count))
   ; (if (> ?count 0)
   ;  then
   ;     (printout t "[" ?obj "].trycount:" ?count crlf)
   ;     (send ?obj put-try-count (- ?count 1))
   ;  else
   ;     (printout t "[" ?obj "]: delete!" crlf)
   ;     (unmake-instance ?obj)
   ; )
)
