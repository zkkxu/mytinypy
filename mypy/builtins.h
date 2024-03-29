ObjType printFunc(TP)
{
    int n = 0;
    ObjType e;
    TP_LOOP(e)
    if (n)
    {
        printf(" ");
    }
    tp_echo(tp, e);
    n += 1;
    TP_END;
    printf("\n");
    return NONE;
}

ObjType bindFunc(TP)
{
    ObjType r = TP_TYPE(FUNCTYPE);
    ObjType self = TP_OBJ();
    return tp_fnc_new(tp,
                      r.fnc.ftype | 2, r.fnc.cfnc, r.fnc.info->code,
                      self, r.fnc.info->globals);
}

ObjType minFunc(TP)
{
    ObjType r = TP_OBJ();
    ObjType e;
    TP_LOOP(e)
    if (compare(tp, r, e) > 0)
    {
        r = e;
    }
    TP_END;
    return r;
}

ObjType maxFunc(TP)
{
    ObjType r = TP_OBJ();
    ObjType e;
    TP_LOOP(e)
    if (compare(tp, r, e) < 0)
    {
        r = e;
    }
    TP_END;
    return r;
}

ObjType copyFunc(TP)
{
    ObjType r = TP_OBJ();
    int type = r.type;
    if (type == LISTTYPE)
    {
        return cp_list(tp, r);
    }
    else if (type == DICTTYPE)
    {
        return _tp_dict_copy(tp, r);
    }
    tp_raise(NONE, mkstring("(copyFunc) TypeError: ?"));
}

ObjType lenFunc(TP)
{
    ObjType e = TP_OBJ();
    return len_func(tp, e);
}

ObjType assertFunc(TP)
{
    int a = TP_NUM();
    if (a)
    {
        return NONE;
    }
    tp_raise(NONE, mkstring("(assertFunc) AssertionError"));
}

ObjType rangeFunc(TP)
{
    int a, b, c, i;
    ObjType r = to_list(tp);
    switch (tp->params.list.val->len)
    {
    case 1:
        a = 0;
        b = TP_NUM();
        c = 1;
        break;
    case 2:
    case 3:
        a = TP_NUM();
        b = TP_NUM();
        c = TP_DEFAULT(number(1)).number.val;
        break;
    default:
        return r;
    }
    if (c != 0)
    {
        for (i = a; (c > 0) ? i < b : i > b; i += c)
        {
            append_list(tp, r.list.val, number(i));
        }
    }
    return r;
}

ObjType sysFunc(TP)
{
    char s[TP_CSTR_LEN];
    tp_cstr(tp, TP_STR(), s, TP_CSTR_LEN);
    int r = system(s);
    return number(r);
}

ObjType isTypeFunc(TP)
{
    ObjType v = TP_OBJ();
    ObjType t = TP_STR();
    if (compare(tp, t, mkstring("string")) == 0)
    {
        return number(v.type == TP_STRING);
    }
    if (compare(tp, t, mkstring("list")) == 0)
    {
        return number(v.type == LISTTYPE);
    }
    if (compare(tp, t, mkstring("dict")) == 0)
    {
        return number(v.type == DICTTYPE);
    }
    if (compare(tp, t, mkstring("number")) == 0)
    {
        return number(v.type == TP_NUMBER);
    }
    if (compare(tp, t, mkstring("fnc")) == 0)
    {
        return number(v.type == FUNCTYPE && (v.fnc.ftype & 2) == 0);
    }
    if (compare(tp, t, mkstring("method")) == 0)
    {
        return number(v.type == FUNCTYPE && (v.fnc.ftype & 2) != 0);
    }
    tp_raise(NONE, mkstring("(is_type) TypeError: ?"));
}

ObjType mkfloat(TP)
{
    ObjType v = TP_OBJ();
    int ord = TP_DEFAULT(number(0)).number.val;
    int type = v.type;
    if (type == TP_NUMBER)
    {
        return v;
    }
    if (type == TP_STRING && v.string.len < 32)
    {
        char s[32];
        memset(s, 0, v.string.len + 1);
        memcpy(s, v.string.val, v.string.len);
        if (strchr(s, '.'))
        {
            return number(atof(s));
        }
        return (number(strtol(s, 0, ord)));
    }
    tp_raise(NONE, mkstring("(mkfloat) TypeError: ?"));
}

ObjType saveFunc(TP)
{
    char fname[256];
    tp_cstr(tp, TP_STR(), fname, 256);
    ObjType v = TP_OBJ();
    FILE *f;
    f = fopen(fname, "wb");
    if (!f)
    {
        tp_raise(NONE, mkstring("(saveFunc) IOError: ?"));
    }
    fwrite(v.string.val, v.string.len, 1, f);
    fclose(f);
    return NONE;
}

ObjType loadFunc(TP)
{
    FILE *f;
    long l;
    ObjType r;
    char *s;
    char fname[256];
    tp_cstr(tp, TP_STR(), fname, 256);
    struct stat stbuf;
    stat(fname, &stbuf);
    l = stbuf.st_size;
    f = fopen(fname, "rb");
    if (!f)
    {
        tp_raise(NONE, mkstring("(loadFunc) IOError: ?"));
    }
    r = to_string(tp, l);
    s = r.string.info->s;
    fread(s, 1, l, f);
    fclose(f);
    return track(tp, r);
}

ObjType fpackFunc(TP)
{
    tp_num v = TP_NUM();
    ObjType r = to_string(tp, sizeof(tp_num));
    *(tp_num *)r.string.val = v;
    return track(tp, r);
}

ObjType absFunc(TP)
{
    return number(fabs(mkfloat(tp).number.val));
}
ObjType intFunc(TP)
{
    return number((long)mkfloat(tp).number.val);
}
tp_num roundfFunc(tp_num v)
{
    tp_num av = fabs(v);
    tp_num iv = (long)av;
    av = (av - iv < 0.5 ? iv : iv + 1);
    return (v < 0 ? -av : av);
}
ObjType roundfunc(TP)
{
    return number(roundfFunc(mkfloat(tp).number.val));
}

ObjType existFunc(TP)
{
    char fname[TP_CSTR_LEN];
    tp_cstr(tp, TP_STR(), fname, TP_CSTR_LEN);
    struct stat stbuf;
    return number(!stat(fname, &stbuf));
}

ObjType mtimeFunc(TP)
{
    char fname[TP_CSTR_LEN];
    tp_cstr(tp, TP_STR(), fname, TP_CSTR_LEN);
    struct stat stbuf;
    if (!stat(fname, &stbuf))
    {
        return number(stbuf.st_mtime);
    }
    tp_raise(NONE, mkstring("(mtimeFunc) IOError: ?"));
}

int _lookup_(TP, ObjType self, ObjType k, ObjType *meta, int depth)
{
    int n = _tp_dict_find(tp, self.dict.val, k);
    if (n != -1)
    {
        *meta = self.dict.val->items[n].val;
        return 1;
    }
    depth--;
    if (!depth)
    {
        tp_raise(0, mkstring("(tp_lookup) RuntimeError: maximum lookup depth exceeded"));
    }
    if (self.dict.dtype && self.dict.val->meta.type == DICTTYPE && _lookup_(tp, self.dict.val->meta, k, meta, depth))
    {
        if (self.dict.dtype == 2 && meta->type == FUNCTYPE)
        {
            *meta = tp_fnc_new(tp, meta->fnc.ftype | 2,
                               meta->fnc.cfnc, meta->fnc.info->code,
                               self, meta->fnc.info->globals);
        }
        return 1;
    }
    return 0;
}

int lookupFunc(TP, ObjType self, ObjType k, ObjType *meta)
{
    return _lookup_(tp, self, k, meta, 8);
}

#define TP_META_BEGIN(self, name)                         \
    if (self.dict.dtype == 2)                             \
    {                                                     \
        ObjType meta;                                     \
        if (lookupFunc(tp, self, mkstring(name), &meta)) \
        {

#define TP_META_END \
    }               \
    }

ObjType setmetaFunc(TP)
{
    ObjType self = TP_TYPE(DICTTYPE);
    ObjType meta = TP_TYPE(DICTTYPE);
    self.dict.val->meta = meta;
    return NONE;
}

ObjType getmetaFunc(TP)
{
    ObjType self = TP_TYPE(DICTTYPE);
    return self.dict.val->meta;
}

ObjType objectFunc(TP)
{
    ObjType self = tp_dict(tp);
    self.dict.dtype = 2;
    return self;
}

ObjType newObjFunc(TP)
{
    ObjType klass = TP_TYPE(DICTTYPE);
    ObjType self = objectFunc(tp);
    self.dict.val->meta = klass;
    TP_META_BEGIN(self, "__init__");
    callfunc(tp, meta, tp->params);
    TP_META_END;
    return self;
}

ObjType objectCallFunc(TP)
{
    ObjType self;
    if (tp->params.list.val->len)
    {
        self = TP_TYPE(DICTTYPE);
        self.dict.dtype = 2;
    }
    else
    {
        self = objectFunc(tp);
    }
    return self;
}

ObjType getrawFunc(TP)
{
    ObjType self = TP_TYPE(DICTTYPE);
    self.dict.dtype = 0;
    return self;
}

ObjType classFunc(TP)
{
    ObjType klass = tp_dict(tp);
    klass.dict.val->meta = get(tp, tp->builtins, mkstring("object"));
    return klass;
}

ObjType boolFunc(TP)
{
    ObjType v = TP_OBJ();
    return (number(mk_bool(tp, v)));
}
