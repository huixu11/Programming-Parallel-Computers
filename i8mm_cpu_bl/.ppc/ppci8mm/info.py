from ppcgrader.info_utils import *
from ppcgrader.doc_builder import *
from ppcgrader.reporter import json_to_output

code = "i8mm"
name = "I8MM"
descr = "integer matrix multiplication"


def explain_web(raw: dict):
    return generate_html(explain(json_to_output(raw), "web"))


def explain_terminal(r, color=False):
    printer = TerminalPrinter(color=color)
    if color:
        hl, minor, reset = '\033[31;1m', '\033[34;1m', '\033[0m'
        printer.set_format("tile correct", minor, reset)
        printer.set_format("tile verywrong", hl, reset)
    else:
        printer.set_format("tile correct", ' ', ' ')
        printer.set_format("tile verywrong", '[', ']')

    return generate_term(explain(r, "term"), printer)


def explain(r, mode: str):

    domain = get_domain(mode=mode, domain=code)

    builder = DocumentBuilder(mode)
    input = r.input_data or {}
    output = r.output_data or {}
    oe = r.output_errors or {}

    m = input.get('m', None)
    n = input.get('n', None)
    k = input.get('k', None)
    tile = input.get('tile_size', None)
    a = input.get('input_a', None)
    b = input.get('input_b', None)
    result = output.get("result", None)
    locations = oe.get("locations", None)

    if m is not None and n is not None and k is not None:
        with builder.text() as txt:
            txt += domain.gettext('function_called_with_params')
        with builder.list(style="compact") as lst:
            lst.add_item(f'm = {m}')
            lst.add_item(f'n = {n}')
            lst.add_item(f'k = {k}')

        if tile is not None:
            m //= tile
            n //= tile
            k //= tile

        if a is not None and b is not None:
            if tile is not None:
                with builder.text() as txt:
                    txt += domain.gettext('input_consisted_of',
                                          tile=tile)  # add old string format
                    txt += domain.gettext('each_number_indicates', tile=tile)
                    txt += domain.gettext('outputs_divided_by', tile=tile)

            with builder.text() as txt:
                txt += domain.gettext('input_data_like')  #

            with builder.list(style="compact") as lst:
                lst.add_item('A')

            with builder.matrix(m, k) as mat:
                for y in range(m):
                    for x in range(k):
                        mat.entry(y, x, safeprint(safeget(a, y, x)))

            with builder.list(style="compact") as lst:
                lst.add_item('B')

            with builder.matrix(k, n) as mat:  # type: MatrixBuilder
                for y in range(k):
                    for x in range(n):
                        mat.entry(y, x, safeprint(safeget(b, y, x)))

        if result is not None:
            with builder.text() as txt:
                txt += domain.gettext('output')
            with builder.matrix(m, n) as mat:
                for y in range(m):
                    for x in range(n):
                        v = safeget(result, y, x)
                        if locations:
                            e = safeget(locations, y, x)
                            if e == 0:
                                mat.entry(y, x, safeprint(v), style="correct")
                            else:
                                mat.entry(y,
                                          x,
                                          safeprint(v),
                                          style="verywrong")
            if locations is not None:
                with builder.text() as txt:
                    txt += domain.gettext('cells_wrong')
        elif locations is not None:
            with builder.text() as txt:
                txt += domain.gettext('results_pattern')
            good = StringNode('·', style="tile correct")
            bad = StringNode('×', style="tile verywrong")
            if mode == "web":
                with builder.matrix(m, n) as mat:
                    for y in range(m):
                        for x in range(n):
                            e = safeget(locations, y, x)
                            if e == 0:
                                mat.entry(y, x, "·", style="correct")
                            else:
                                mat.entry(y, x, "×", style="verywrong")
            else:
                with builder.text() as txt:
                    for y in range(m):
                        txt += ' '
                        for x in range(n):
                            e = safeget(locations, y, x)
                            if e == 0:
                                txt += good
                            else:
                                txt += bad
                        txt += '\n'
                    txt += '\n'
            with builder.text() as txt:
                txt += domain.gettext('highlighted_follows')
            with builder.list(style="compact") as lst:
                lst.add_item(good + ' — ' + domain.gettext('correct result'))
                lst.add_item(bad + ' — ' + domain.gettext('wrong result'))
        else:
            with builder.text() as txt:
                txt += domain.gettext('probabilistic_error')

    return builder.build()
