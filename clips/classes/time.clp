(defclass DateTime
    (is-a USER)
    (slot year
        (type INTEGER)
        (default 2018)
    )
    (slot month
        (type INTEGER)
        (default 01)
    )
    (slot day
        (type INTEGER)
        (default 01)
    )
    (slot hour
        (type INTEGER)
        (default 0)
    )
    (slot minute
        (type INTEGER)
        (default 0)
    )
    (slot second
        (type INTEGER)
        (default 01)
    )
)
