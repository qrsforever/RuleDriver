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
;	Global Class
;-----------------------------------------------------------------

; Base Device Abstract
(defclass DEVICE (is-a USER)
    (slot ID     (visibility public) (type SYMBOL))
    (slot Class  (visibility public) (type SYMBOL))
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

(deffunction act_control (?id ?slot ?value)
    (printout debug "act_control(" ?id "," ?slot "," ?value ")" crlf)    
)

(deffunction act_notify (?title ?message)
    (printout debug "act_notify(" ?title "," ?message ")" crlf) 
)

(deffunction act_scene (?type $?ruleid-list)
    (foreach ?ruleid (create$ $?ruleid-list)
        (printout debug "act_scene(" ?ruleid ")" crlf)
    )
)
