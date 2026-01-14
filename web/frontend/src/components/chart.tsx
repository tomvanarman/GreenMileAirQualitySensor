import { useEffect, useRef, useState } from 'react';

const STROKE_MAX = 314;

const colorThresholds = [
    { color: 'rgb(0, 255, 0)', description: 'Normal' },
    { color: 'rgb(255, 255, 0)', description: 'Moderate' },
    { color: 'rgb(255, 165, 0)', description: 'Unhealthy for sensitive groups' },
    { color: 'rgb(255, 0, 0)', description: 'Unhealthy' },
    { color: 'rgb(128, 0, 0)', description: 'Very Uhealthy' },
];

export type ChartProps = {
    percent: number;
    isLoading: boolean;
};

export function Chart({ percent, isLoading }: ChartProps) {
    const pathRef = useRef<SVGPathElement>(null);
    const [colorIndex, setColorIndex] = useState(0);

    useEffect(() => {
        if (!pathRef.current || isLoading) return;

        const percentToPixel = STROKE_MAX - (percent / 100) * STROKE_MAX;

        let index = Math.floor((percent / 100) * colorThresholds.length);
        if (index >= colorThresholds.length) index = colorThresholds.length - 1;
        (console.log(index), percentToPixel);
        // pathRef.current.style.stroke = colorThresholds[index];
        setColorIndex(index);

        pathRef.current.style.strokeDashoffset = `${percentToPixel}px`;
    }, [pathRef.current, percent, setColorIndex]);

    return (
        <div className='w-full flex flex-col mb-4'>
            <svg xmlns='http://www.w3.org/2000/svg' className='donut-svg' viewBox='0 0 300 200'>
                <path
                    d='M 50 150 A 100 100 0 0 1 250 150'
                    fill='none'
                    stroke='#3d3d3d'
                    strokeWidth='25'
                    strokeLinecap='round'
                    strokeLinejoin='round'
                />
                <path
                    ref={pathRef}
                    d='M 50 150 A 100 100 0 0 1 250 150'
                    fill='none'
                    stroke={isLoading ? 'rgb(0, 0, 255)' : colorThresholds[colorIndex].color}
                    strokeWidth='25'
                    strokeLinecap='round'
                    strokeLinejoin='round'
                    className='stroke-animate'
                />
            </svg>
            {isLoading ? (
                <>
                    <p className='w-full text-center font-bold text-xl'>
                        Air Pollution: Measuring...
                    </p>
                    <p className='w-full text-center text-xl'> Air Quality: ...</p>
                </>
            ) : (
                <>
                    <p className='w-full text-center font-bold text-xl'>
                        Air Pollution: {percent}%
                    </p>
                    <p className='w-full text-center text-xl'>
                        Air Quality: {colorThresholds[colorIndex].description}
                    </p>
                </>
            )}
        </div>
    );
}
