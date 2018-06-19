;=================================================================
; date: 2018-06-15 15:32:09
; title: class-001
;=================================================================

; A light device
(defclass LIGHT-DEVICE "a light device"
    (is-a DEVICE) (role concrete) (pattern-match reactive) 

    (slot switch 
        (type SYMBOL) 
        (allowed-symbols ON OFF)
        (default OFF)
    )
)
