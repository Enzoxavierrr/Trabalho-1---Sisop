#!/usr/bin/env python3
"""Gera um CSV de medianas e gráficos SVG a partir de resultados.csv."""

import csv
import html
import math
import statistics
from collections import defaultdict
from pathlib import Path


BASE_DIR = Path(__file__).resolve().parent
INPUT_CSV = BASE_DIR / "resultados.csv"
SUMMARY_CSV = BASE_DIR / "amostras_resumo.csv"
TOTAL_SVG = BASE_DIR / "grafico_tempo_total.svg"
PHASES_SVG = BASE_DIR / "grafico_fases.svg"

TIME_FIELDS = (
    "tempo_ms",
    "preparo_ms",
    "fase2_ms",
    "fase3_ms",
    "fase4_ms",
)
RAW_PHASE_FIELDS = ("preparo_ms", "fase2_ms", "fase3_ms", "fase4_ms")
PHASES = (
    ("preparo_mediano_ms", "Preparo", "#8c9aa8"),
    ("fase2_mediana_ms", "Fase 2", "#3677b5"),
    ("fase3_mediana_ms", "Fase 3", "#e28a35"),
    ("fase4_mediana_ms", "Fase 4", "#55a868"),
)
REQUIRED_COLUMNS = {
    "matriz",
    "dimensoes",
    "versao",
    "n_threads",
    "grade",
    "rep",
    "tempo_ms",
    "objetos",
    "preparo_ms",
    "fase2_ms",
    "fase3_ms",
    "fase4_ms",
}
COLORS = ("#3677b5", "#e28a35", "#55a868", "#8b6bb1", "#d64f64")


def number(value, field, line_number, allow_blank=False):
    if value == "" and allow_blank:
        return None
    try:
        return float(value)
    except ValueError as error:
        raise ValueError(
            "Valor inválido na linha {} da coluna {}: {!r}".format(
                line_number, field, value
            )
        ) from error


def read_samples():
    groups = defaultdict(list)
    sample_order = []
    with INPUT_CSV.open("r", encoding="utf-8-sig", newline="") as source:
        reader = csv.DictReader(source)
        missing = REQUIRED_COLUMNS.difference(reader.fieldnames or ())
        if missing:
            raise ValueError(
                "Colunas ausentes em {}: {}".format(
                    INPUT_CSV.name, ", ".join(sorted(missing))
                )
            )

        for line_number, row in enumerate(reader, start=2):
            sample = row["matriz"]
            key = (sample, row["versao"], int(number(
                row["n_threads"], "n_threads", line_number
            )))
            if sample not in sample_order:
                sample_order.append(sample)

            parsed = dict(row)
            for field in TIME_FIELDS:
                parsed[field] = number(
                    row[field], field, line_number, allow_blank=True
                )
            parsed["objetos"] = int(number(
                row["objetos"], "objetos", line_number
            ))
            groups[key].append(parsed)

    if not groups:
        raise ValueError("{} não contém amostras.".format(INPUT_CSV.name))
    return groups, sample_order


def median_for(rows, field):
    values = [row[field] for row in rows if row[field] is not None]
    return statistics.median(values) if values else None


def format_number(value):
    return "" if value is None else "{:.3f}".format(value)


def summarize(groups, sample_order):
    summary = []
    for sample in sample_order:
        keys = [key for key in groups if key[0] == sample]
        seq_keys = [key for key in keys if key[1] == "seq"]
        if not seq_keys:
            raise ValueError("A amostra {} não tem execução sequencial.".format(sample))
        sequential_ms = median_for(groups[seq_keys[0]], "tempo_ms")

        for key in sorted(keys, key=lambda item: (item[1] != "seq", item[2])):
            rows = groups[key]
            first = rows[0]
            object_counts = {row["objetos"] for row in rows}
            if len(object_counts) != 1:
                raise ValueError(
                    "A contagem de objetos varia entre repetições de {}.".format(sample)
                )

            if key[1] == "seq":
                elapsed = median_for(rows, "tempo_ms")
            else:
                combined_times = []
                for row in rows:
                    phase_times = [row[field] for field in RAW_PHASE_FIELDS]
                    if any(value is None for value in phase_times):
                        raise ValueError(
                            "Fase sem tempo na amostra {} ({} threads, repetição {}).".format(
                                sample, key[2], row["rep"]
                            )
                        )
                    combined_times.append(sum(phase_times))
                elapsed = statistics.median(combined_times)
            speedup = sequential_ms / elapsed
            parallel = key[1] == "par"
            summary.append({
                "matriz": sample,
                "dimensoes": first["dimensoes"],
                "versao": key[1],
                "n_threads": key[2],
                "grade": first["grade"],
                "repeticoes": len(rows),
                "tempo_mediano_ms": elapsed,
                "tempo_csv_mediano_ms": median_for(rows, "tempo_ms"),
                "objetos": first["objetos"],
                "speedup_vs_seq": speedup,
                "eficiencia": speedup / key[2] if parallel else None,
                "preparo_mediano_ms": median_for(rows, "preparo_ms"),
                "fase2_mediana_ms": median_for(rows, "fase2_ms"),
                "fase3_mediana_ms": median_for(rows, "fase3_ms"),
                "fase4_mediana_ms": median_for(rows, "fase4_ms"),
            })
    return summary


def write_summary(summary):
    fields = (
        "matriz",
        "dimensoes",
        "versao",
        "n_threads",
        "grade",
        "repeticoes",
        "tempo_mediano_ms",
        "tempo_csv_mediano_ms",
        "objetos",
        "speedup_vs_seq",
        "eficiencia",
        "preparo_mediano_ms",
        "fase2_mediana_ms",
        "fase3_mediana_ms",
        "fase4_mediana_ms",
    )
    with SUMMARY_CSV.open("w", encoding="utf-8", newline="") as target:
        writer = csv.DictWriter(target, fieldnames=fields)
        writer.writeheader()
        for row in summary:
            writer.writerow({
                field: (
                    format_number(row[field])
                    if field.endswith("_ms")
                    or field in ("speedup_vs_seq", "eficiencia")
                    else row[field]
                )
                for field in fields
            })


def sample_label(sample):
    label = Path(sample).stem.replace("_", " ")
    return label.replace("x", "×")


def svg_text(x, y, text, size=12, anchor="middle", color="#273444", weight="normal"):
    return (
        '<text x="{:.1f}" y="{:.1f}" text-anchor="{}" '
        'font-family="Arial, sans-serif" font-size="{}" fill="{}" '
        'font-weight="{}">{}</text>'.format(
            x, y, anchor, size, color, weight, html.escape(str(text))
        )
    )


def nice_max(value):
    if value <= 0:
        return 1
    magnitude = 10 ** math.floor(math.log10(value))
    scaled = value / magnitude
    for step in (1, 2, 2.5, 5, 10):
        if scaled <= step:
            return step * magnitude
    return 10 * magnitude


def svg_start(title, subtitle, width, height):
    return [
        '<svg xmlns="http://www.w3.org/2000/svg" width="{}" height="{}" '
        'viewBox="0 0 {} {}">'.format(width, height, width, height),
        "<title>{}</title>".format(html.escape(title)),
        '<rect width="100%" height="100%" fill="#ffffff"/>',
        svg_text(width / 2, 28, title, 21, weight="bold"),
        svg_text(width / 2, 49, subtitle, 12, color="#596775"),
    ]


def panel_layout(index, columns, panel_width, panel_height):
    return (
        28 + (index % columns) * panel_width,
        68 + (index // columns) * panel_height,
    )


def write_total_chart(summary, sample_order):
    columns = 3
    panel_width = 390
    panel_height = 315
    rows_count = math.ceil(len(sample_order) / columns)
    width = 28 + columns * panel_width
    height = 70 + rows_count * panel_height
    parts = svg_start(
        "Tempo combinado por configuração",
        "Preparo + Fases 2–4 (mediana por execução); cada amostra usa escala própria",
        width,
        height,
    )
    summary_by_sample = defaultdict(list)
    for row in summary:
        summary_by_sample[row["matriz"]].append(row)

    for index, sample in enumerate(sample_order):
        panel_x, panel_y = panel_layout(index, columns, panel_width, panel_height)
        panel = sorted(
            summary_by_sample[sample],
            key=lambda row: (row["versao"] != "seq", row["n_threads"]),
        )
        values = [row["tempo_mediano_ms"] for row in panel]
        maximum = nice_max(max(values) * 1.08)
        left = panel_x + 49
        top = panel_y + 42
        plot_width = panel_width - 72
        plot_height = panel_height - 92
        parts.append(svg_text(
            panel_x + panel_width / 2, panel_y + 19,
            sample_label(sample), 14, weight="bold"
        ))

        for tick in range(5):
            value = maximum * tick / 4
            y = top + plot_height * (1 - tick / 4)
            parts.append(
                '<line x1="{:.1f}" y1="{:.1f}" x2="{:.1f}" y2="{:.1f}" '
                'stroke="#dce2e8" stroke-width="1"/>'.format(
                    left, y, left + plot_width, y
                )
            )
            parts.append(svg_text(left - 7, y + 4, "{:g}".format(value), 10, "end"))

        slot = plot_width / len(panel)
        bar_width = min(30, slot * 0.62)
        for position, data in enumerate(panel):
            bar_height = data["tempo_mediano_ms"] / maximum * plot_height
            x = left + slot * position + (slot - bar_width) / 2
            y = top + plot_height - bar_height
            color = "#778899" if data["versao"] == "seq" else COLORS[position - 1]
            parts.append(
                '<rect x="{:.1f}" y="{:.1f}" width="{:.1f}" height="{:.1f}" '
                'rx="2" fill="{}"><title>{}: {:.3f} ms; {} repetições</title></rect>'.format(
                    x, y, bar_width, bar_height, color,
                    "Sequencial" if data["versao"] == "seq" else
                    "{} threads".format(data["n_threads"]),
                    data["tempo_mediano_ms"], data["repeticoes"],
                )
            )
            label = "Seq" if data["versao"] == "seq" else "{}t".format(data["n_threads"])
            parts.append(svg_text(x + bar_width / 2, top + plot_height + 17, label, 10))

        parts.append(svg_text(
            panel_x + 13, top + plot_height / 2,
            "ms", 10, "middle", color="#596775"
        ))

    parts.append("</svg>")
    TOTAL_SVG.write_text("\n".join(parts) + "\n", encoding="utf-8")


def write_phases_chart(summary, sample_order):
    columns = 3
    panel_width = 390
    panel_height = 335
    rows_count = math.ceil(len(sample_order) / columns)
    width = 28 + columns * panel_width
    height = 98 + rows_count * panel_height
    parts = svg_start(
        "Tempos medianos por fase",
        "Medianas independentes por fase (ms); as barras não representam a mediana do total",
        width,
        height,
    )
    legend_y = 71
    legend_x = max(45, (width - 470) / 2)
    for index, (field, label, color) in enumerate(PHASES):
        x = legend_x + index * 118
        parts.append(
            '<rect x="{:.1f}" y="{}" width="12" height="12" fill="{}"/>'.format(
                x, legend_y - 10, color
            )
        )
        parts.append(svg_text(x + 18, legend_y, label, 11, "start"))

    summary_by_sample = defaultdict(list)
    for row in summary:
        if row["versao"] == "par":
            summary_by_sample[row["matriz"]].append(row)

    for index, sample in enumerate(sample_order):
        panel_x, panel_y = panel_layout(index, columns, panel_width, panel_height)
        panel = sorted(summary_by_sample[sample], key=lambda row: row["n_threads"])
        totals = [
            sum(row[field] or 0 for field, _, _ in PHASES)
            for row in panel
        ]
        maximum = nice_max(max(totals) * 1.08)
        left = panel_x + 49
        top = panel_y + 42
        plot_width = panel_width - 72
        plot_height = panel_height - 103
        parts.append(svg_text(
            panel_x + panel_width / 2, panel_y + 19,
            sample_label(sample), 14, weight="bold"
        ))

        for tick in range(5):
            value = maximum * tick / 4
            y = top + plot_height * (1 - tick / 4)
            parts.append(
                '<line x1="{:.1f}" y1="{:.1f}" x2="{:.1f}" y2="{:.1f}" '
                'stroke="#dce2e8" stroke-width="1"/>'.format(
                    left, y, left + plot_width, y
                )
            )
            parts.append(svg_text(left - 7, y + 4, "{:g}".format(value), 10, "end"))

        slot = plot_width / max(1, len(panel))
        bar_width = min(34, slot * 0.60)
        for position, data in enumerate(panel):
            x = left + slot * position + (slot - bar_width) / 2
            y_bottom = top + plot_height
            phase_values = []
            for field, label, color in PHASES:
                value = data[field] or 0
                phase_values.append(value)
                bar_height = value / maximum * plot_height
                y_bottom -= bar_height
                parts.append(
                    '<rect x="{:.1f}" y="{:.1f}" width="{:.1f}" height="{:.1f}" '
                    'fill="{}"><title>{} threads — {}: {:.3f} ms</title></rect>'.format(
                        x, y_bottom, bar_width, bar_height, color,
                        data["n_threads"], label, value,
                    )
                )
            parts.append(svg_text(
                x + bar_width / 2, top + plot_height + 17,
                "{}t".format(data["n_threads"]), 10
            ))

    parts.append("</svg>")
    PHASES_SVG.write_text("\n".join(parts) + "\n", encoding="utf-8")


def main():
    groups, sample_order = read_samples()
    summary = summarize(groups, sample_order)
    write_summary(summary)
    write_total_chart(summary, sample_order)
    write_phases_chart(summary, sample_order)
    print("CSV resumido: {}".format(SUMMARY_CSV))
    print("Gráfico de tempo total: {}".format(TOTAL_SVG))
    print("Gráfico por fase: {}".format(PHASES_SVG))
    print(
        "{} amostras, {} configurações resumidas.".format(
            len(sample_order), len(summary)
        )
    )


if __name__ == "__main__":
    main()
