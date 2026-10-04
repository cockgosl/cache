import pandas as pd
import matplotlib.pyplot as plt
import seaborn as sns
import os
import sys

# Настройки стиля для красивых графиков
plt.style.use('seaborn-v0_8-whitegrid')
plt.rcParams['font.family'] = 'sans-serif'

def main():
    # 1. Поиск файла benchmarks.csv
    file_path = 'benchmarks.csv'
    if not os.path.exists(file_path):
        if os.path.exists('benchmarks/benchmarks.csv'):
            file_path = 'benchmarks/benchmarks.csv'
        else:
            print("Ошибка: Файл benchmarks.csv не найден!")
            print("Сначала запустите скомпилированный C++ файл бенчмарка.")
            sys.exit(1)

    print(f"Чтение данных из {file_path}...")
    df = pd.read_csv(file_path)

    # Объединяем архитектуру и алгоритм для красивых подписей (например: "Multi_Inclusive: ARC+LRU")
    df['Конфигурация'] = df['Architecture'].str.replace('Multi_', '') + " | " + df['Algorithm']
    df.loc[df['Architecture'] == 'Single', 'Конфигурация'] = "Single | " + df['Algorithm']

    # =========================================================================
    # ГРАФИК 1: Тепловая карта (Heatmap) как на первом фото
    # =========================================================================
    print("Генерация тепловой карты...")

    # Создаем сводную таблицу (Pivot Table)
    heatmap_data = df.pivot_table(
        index='Конфигурация',
        columns='Workload',
        values='Efficiency_%'
    )

    # Сортируем индекс по среднему значению, чтобы лучшие были сверху
    heatmap_data['Mean'] = heatmap_data.mean(axis=1)
    heatmap_data = heatmap_data.sort_values(by='Mean', ascending=False).drop(columns=['Mean'])

    plt.figure(figsize=(14, 10))

    # Используем цветовую схему viridis_r (максимально похожа на желто-зеленую с фото)
    ax = sns.heatmap(
        heatmap_data,
        annot=True,          # Показывать цифры в ячейках
        fmt=".1f",           # Формат чисел (1 знак после запятой)
        cmap="viridis_r",    # Цветовая палитра
        vmin=0, vmax=100,    # Границы шкалы от 0 до 100%
        cbar_kws={'label': '% от идеала Belady'},
        linewidths=.5
    )

    plt.title("Эффективность алгоритмов кэширования относительно идеала (Belady's OPT)", fontsize=16, pad=20)
    plt.xlabel("Тип нагрузки (Workload)", fontsize=12)
    plt.ylabel("Архитектура | Алгоритм", fontsize=12)
    plt.xticks(rotation=45, ha='right')
    plt.tight_layout()
    plt.savefig("01_heatmap_efficiency.png", dpi=300)
    print("Сохранено: 01_heatmap_efficiency.png")

    # =========================================================================
    # ГРАФИК 2: Топ-10 конфигураций (Bar Chart) как на втором фото
    # =========================================================================
    print("Генерация графика Топ-10...")

    # Считаем среднюю эффективность по всем типам нагрузок
    avg_efficiency = df.groupby('Конфигурация')['Efficiency_%'].mean().reset_index()
    top10 = avg_efficiency.sort_values(by='Efficiency_%', ascending=False).head(10)

    plt.figure(figsize=(12, 6))

    # Рисуем горизонтальный график
    bars = plt.barh(top10['Конфигурация'], top10['Efficiency_%'], color='#4c72b0')
    plt.gca().invert_yaxis() # Переворачиваем ось, чтобы #1 был сверху

    # Добавляем цифры прямо внутрь столбцов
    for bar in bars:
        width = bar.get_width()
        # Позиционируем текст немного левее края столбца
        plt.text(
            width - 2,
            bar.get_y() + bar.get_height() / 2,
            f'{width:.1f}%',
            va='center', ha='right', color='white', fontweight='bold', fontsize=11
        )

    plt.title("Десять лучших конфигураций кэша (среднее % от Belady по всем тестам)", fontsize=15, pad=15)
    plt.xlabel("Средние попадания относительно Belady, %", fontsize=12)
    plt.ylabel("", fontsize=12)
    plt.xlim(0, 100)

    # Вертикальные линии сетки для удобства чтения
    plt.grid(axis='x', linestyle='--', alpha=0.7)

    plt.tight_layout()
    plt.savefig("02_top10_algorithms.png", dpi=300)
    print("Сохранено: 02_top10_algorithms.png")

    # Показать графики на экране
    plt.show()

if __name__ == "__main__":
    main()
