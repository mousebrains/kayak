def getParameter(cur, ident:str) -> str:
    cur.execute("SELECT value FROM parameters WHERE ident=%s;", (ident,))
    for row in cur: return row[0]
    return None
